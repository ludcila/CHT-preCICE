from preciceautoconf.interface import *
from preciceautoconf.participant import *
from preciceautoconf.xmlns import *
from preciceautoconf.rules import *
from preciceautoconf.schemes import *

# Create participants

participants = []

innerFluid = ParticipantFactory.getParticipant("OpenFOAM", "Inner-Fluid")
innerFluidInterface = innerFluid.addInterface()
participants.append(innerFluid)

outerFluid = ParticipantFactory.getParticipant("OpenFOAM", "Outer-Fluid")
outerFluidInterface = outerFluid.addInterface()
participants.append(outerFluid)

solid = ParticipantFactory.getParticipant("Code_Aster", "Solid")
outerSolidInterface = solid.addInterface()
innerSolidInterface = solid.addInterface()
participants.append(solid)

# Set matching interfaces

outerSolidInterface.setPartnerInterface(outerFluidInterface)
innerSolidInterface.setPartnerInterface(innerFluidInterface)

# Create couplings
# TODO: automatically generate this
couplings = [[solid, innerFluid], [solid, outerFluid]]

# Common parameters

timestep = 0.01
maxTime = 1.0
maxIterations = 30

# Determine type of coupling configuration to be used

config = CouplingConfiguration(
    couplings=couplings,
    steadyState=False,
    forceExplicit=False,
    forceParallel=True
)

# --------------------------------------------------------------------------------
#   Create XML tree
# --------------------------------------------------------------------------------
nsmap = {
    'data': "data",
    'mapping': "mapping",
    'coupling-scheme': "coupling-scheme",
    'post-processing': "post-processing"
}
preciceConfigurationTag = etree.Element("precice-configuration", nsmap=nsmap)

for participant in participants:
    participant.addDataTagsTo(preciceConfigurationTag)

for participant in participants:
    participant.addMeshTagsTo(preciceConfigurationTag)

for participant in participants:
    participant.addParticipantTagTo(preciceConfigurationTag)

if config.multi():
    # If multi, all couplings are treated together
    couplingScheme = MultiCouplingScheme(timestep, maxTime, maxIterations, couplings)
    couplingScheme.addCouplingSchemeTagTo(preciceConfigurationTag)

else:
    # If not multi, couplings are treated per pair
    for participantsPair in couplings:
        # Determine first and second participant
        if config.getColors()[participantsPair[0]] == 1:
            participantsPair.reverse()
        if config.implicit():
            couplingScheme = ImplicitCouplingScheme(timestep, maxTime, maxIterations, participantsPair, serial=config.serial())
        else:
            couplingScheme = CouplingScheme(timestep, maxTime, participantsPair, serial=config.serial())
        couplingScheme.addCouplingSchemeTagTo(preciceConfigurationTag)

print etree.tostring(preciceConfigurationTag, pretty_print=True)
