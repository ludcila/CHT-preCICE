import yaml
from preciceautoconf.interface import *
from preciceautoconf.participant import *
from preciceautoconf.xmlns import *
from preciceautoconf.rules import *
from preciceautoconf.schemes import *

# Create participants and couplings from YAML file

file = open("config.yml")
input = yaml.load(file.read())

participants = []
interfacesMap = {}
participantsList = input["participants"]
for participantName in participantsList:
    participantData = participantsList[participantName]
    participant = ParticipantFactory.getParticipant(participantData["solver"], participantName)
    participants.append(participant)
    for interfaceName in participantData["interfaces"]:
        interface = participant.addInterface(interfaceName)
        interfacesMap[interfaceName] = interface

couplingsList = input["couplings"]
for coupling in couplingsList:
    interface1 = interfacesMap[coupling[0]]
    interface2 = interfacesMap[coupling[1]]
    interface1.setPartnerInterface(interface2)

couplings = []
for i in range(len(participants)):
    for j in range(i, len(participants)):
        if participants[i].hasInterfacesWith(participants[j]):
            couplings.append([participants[i], participants[j]])

# Simulation parameters

timestep = input["simulation"]["timestep"]
maxTime = input["simulation"]["maxTime"]
maxIterations = input["simulation"]["maxCouplingIterations"]

# Determine type of coupling configuration to be used

config = CouplingConfiguration(
    couplings=couplings,
    steadyState=input["simulation"]["steadyState"],
    forceExplicit=input["simulation"]["forceExplicit"],
    forceParallel=input["simulation"]["forceParallel"]
)

# --------------------------------------------------------------------------------
#   Create XML tree
# --------------------------------------------------------------------------------

nsmap = {
    "data": "data",
    "mapping": "mapping",
    "coupling-scheme": "coupling-scheme",
    "post-processing": "post-processing",
    "m2n": "m2n",
    "master": "master"
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

    # Add tags
    couplingScheme.addM2nTagTo(preciceConfigurationTag)
    couplingScheme.addCouplingSchemeTagTo(preciceConfigurationTag)

else:

    # If not multi, couplings are treated per pair
    for participantsPair in couplings:

        # Determine first and second participant
        config.sortParticipants(participantsPair)

        # Create implicit or explicit coupling scheme
        if config.implicit():
            couplingScheme = ImplicitCouplingScheme(timestep, maxTime, maxIterations, participantsPair, serial=config.serial())
        else:
            couplingScheme = CouplingScheme(timestep, maxTime, participantsPair, serial=config.serial())

        # Add tags
        couplingScheme.addM2nTagTo(preciceConfigurationTag)
        couplingScheme.addCouplingSchemeTagTo(preciceConfigurationTag)

print etree.tostring(preciceConfigurationTag, pretty_print=True)
