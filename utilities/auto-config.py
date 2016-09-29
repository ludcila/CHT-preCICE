from preciceautoconf.interface import *
from preciceautoconf.participant import *
from preciceautoconf.xmlns import *
from preciceautoconf.rules import *
from preciceautoconf.schemes import *

# Create participants

participants = []

innerFluid = OpenFOAMParticipant("Inner-Fluid")
innerFluidInterface = innerFluid.addInterface()
participants.append(innerFluid)

outerFluid = OpenFOAMParticipant("Outer-Fluid")
outerFluidInterface = outerFluid.addInterface()
participants.append(outerFluid)

solid = CodeAsterParticipant("Solid")
outerSolidInterface = solid.addInterface()
innerSolidInterface = solid.addInterface()
participants.append(solid)

# Set matching interfaces

outerSolidInterface.setPartnerInterface(outerFluidInterface)
innerSolidInterface.setPartnerInterface(innerFluidInterface)

# Create couplings
# TODO: automatically generate this
couplings = [[solid, innerFluid], [solid, outerFluid]]

# Common paramters

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

preciceConfigurationTag = etree.Element("precice-configuration", nsmap=XMLNamespaces.nsmap)

for participant in participants:
    participant.addDataTags(preciceConfigurationTag)

for participant in participants:
    participant.addMeshTags(preciceConfigurationTag)

for participant in participants:
    participant.addParticipantTag(preciceConfigurationTag)

if config.multi():
    # If multi, all couplings are treated together
    couplingScheme = MultiCouplingScheme(timestep, maxTime, maxIterations, couplings)
    couplingScheme.addCouplingSchemeTag(preciceConfigurationTag)

else:
    # If not multi, couplings are treated per pair
    for participantsPair in couplings:
        # Determine first and second participant
        if config.getColors()[participantsPair[0]] == 1:
            participantsPair.reverse()
        if config.implicit():
            couplingScheme = ImplicitCouplingScheme(timestep, maxTime, maxIterations, participantsPair, serial=serial)
        else:
            couplingScheme = CouplingScheme(timestep, maxTime, participantsPair, serial=serial)
        couplingScheme.addCouplingSchemeTag(preciceConfigurationTag)

print etree.tostring(preciceConfigurationTag, pretty_print=True)
