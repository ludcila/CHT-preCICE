import matplotlib.pyplot as plt
import networkx as nx

class Interface(object):

    def __init__(self, participant):
        self.participant = participant

    def setPartnerInterface(self, partnerInterface):
        self.partnerInterface = partnerInterface
        if not hasattr(partnerInterface, "partnerInterface"):
            partnerInterface.setPartnerInterface(self)
        self.setMeshNames()

    def setMeshNames(self):
        pass

    def getProvideMeshTags(self):
        print '\t<use-mesh name="' + self.readMesh + '" provide="yes"/>'
        if self.readMesh != self.writeMesh:
            print '\t<use-mesh name="' + self.writeMesh + '" provide="yes"/>'

    def getFromMeshTag(self):
        print '\t<use-mesh name="' + self.writeMesh + '" from="' + self.participant.name + '"/>'

    def getReadWriteMappingTags(self):
        print '\t<write-data name="' + self.participant.dataNameT + '" mesh="' + self.writeMesh + '"/>'
        print '\t<write-data name="' + self.participant.dataNameHTC + '" mesh="' + self.writeMesh + '"/>'
        print '\t<read-data name="' + self.partnerInterface.participant.dataNameT + '" mesh="' + self.readMesh + '"/>'
        print '\t<read-data name="' + self.partnerInterface.participant.dataNameHTC + '" mesh="' + self.readMesh + '"/>'
        print '\t<mapping:nearest-neighbor direction="read" from="' + self.partnerInterface.writeMesh + '" to="' + self.readMesh + '"/>'

    def getExchangeTags(self, initialize=False):
        # From me to partner
        print '\t<exchange data="' + self.participant.dataNameT + '" mesh="' + self.writeMesh + '" from="' + self.participant.name + '" to="' + self.partnerInterface.participant.name + '"/>'
        print '\t<exchange data="' + self.participant.dataNameHTC + '" mesh="' + self.writeMesh + '" from="' + self.participant.name + '" to="' + self.partnerInterface.participant.name + '"/>'

    def getPostProcessingDataTags(self):
        pass

class OpenFOAMInterface(Interface):

    def __init__(self, participant):
        super(OpenFOAMInterface, self).__init__(participant)

    def getMeshTags(self):
        print '<mesh name="' + self.mesh + '"/>'
        print '\t<use-data name="' + self.participant.dataNameT + '"/>'
        print '\t<use-data name="' + self.participant.dataNameHTC + '"/>'
        print '\t<use-data name="' + self.partnerInterface.participant.dataNameT + '"/>'
        print '\t<use-data name="' + self.partnerInterface.participant.dataNameHTC + '"/>'
        print '<mesh/>'

    def setMeshNames(self):
        self.mesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Faces-Mesh"
        self.readMesh = self.mesh
        self.writeMesh = self.mesh

class CodeAsterInterface(Interface):

    def __init__(self, participant):
        super(CodeAsterInterface, self).__init__(participant)

    def getMeshTags(self):
        print '<mesh name="' + self.writeMesh + '"/>'
        print '\t<use-data name="' + self.participant.dataNameT + '"/>'
        print '\t<use-data name="' + self.participant.dataNameHTC + '"/>'
        print '<mesh/>'
        print '<mesh name="' + self.readMesh + '"/>'
        print '\t<use-data name="' + self.partnerInterface.participant.dataNameT + '"/>'
        print '\t<use-data name="' + self.partnerInterface.participant.dataNameHTC + '"/>'
        print '<mesh/>'

    def setMeshNames(self):
        self.readMesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Faces-Mesh"
        self.writeMesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Nodes-Mesh"

class Participant(object):

    def __init__(self, name):
        self.name = name
        self.interfaces = []
        self.dataNameT = "Sink-Temperature-" + self.name
        self.dataNameHTC = "Heat-Transfer-Coefficient-" + self.name

    def getDataTags(self):
        print '<data:scalar name="' + self.dataNameHTC + '"/>'
        print '<data:scalar name="' + self.dataNameT + '"/>'

    def getMeshTags(self):
        for interface in self.interfaces:
            interface.getMeshTags()

    def getParticipantTag(self):
        print '<participant name="' + self.name + '"/>'
        for interface in self.interfaces:
            interface.getProvideMeshTags()
            interface.partnerInterface.getFromMeshTag()
            interface.getReadWriteMappingTags()
        print '</participant>'

    def getInterfacesWith(self, partner):
        interfaces = []
        for interface in self.interfaces:
            if interface.partnerInterface.participant == partner:
                interfaces.append(interface)
        return interfaces

class OpenFOAMParticipant(Participant):

    def __init__(self, name):
        super(OpenFOAMParticipant, self).__init__(name)
        self.solverType = "OpenFOAM"

    def addInterface(self):
        interface = OpenFOAMInterface(self)
        self.interfaces.append(interface)
        return interface

class CodeAsterParticipant(Participant):

    def __init__(self, name):
        super(CodeAsterParticipant, self).__init__(name)
        self.solverType = "CodeAster"

    def addInterface(self):
        interface = CodeAsterInterface(self)
        self.interfaces.append(interface)
        return interface

class CouplingScheme(object):
    def __init__(self, timestep, maxTime, participants, serial=False):
        self.timestep = timestep
        self.maxTime = maxTime
        self.participants = participants
        if not serial:
            scheme = "serial"
        else:
            scheme = "parallel"
        self.type = "explicit"
        self.scheme = scheme + "-" + self.type

    def getCouplingSchemeTag(self):
        print "<coupling-scheme:" + self.scheme + ">"

    def getCouplingParticipantTags(self):
        print '<participants first="' + self.participants[0].name + '"' ' second="' + self.participants[1].name + '"/>'

    def getExchangeTags(self):
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            interface.getExchangeTags()
            interface.partnerInterface.getExchangeTags()
            interface.getPostProcessingDataTags()


class ImplicitCouplingScheme(CouplingScheme):
    def __init__(self, timestep, maxTime, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
    def getRelativeConvergenceMeasureTags(self):
        pass
    def getMaxIterationsTags(self):
        pass
    def getPostProcessingDataTagsForParticipantTag(self, participant):
        pass

class MultiCouplingScheme(ImplicitCouplingScheme):

    def __init__(self, timestep, maxTime, participantPairs, serial=False):
        self.participantPairs = participantPairs
        self.participants = []
        for pair in self.participantPairs:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))
        super(MultiCouplingScheme, self).__init__(timestep, maxTime, self.participants, False)

    def getCouplingParticipantTags(self):
        for participant in self.participants:
            print '<participant name="' + participant.name + '"/>'

    def getExchangeTags(self):
        for pair in self.participantPairs:
            interfaces = pair[0].getInterfacesWith(pair[1])
            for interface in interfaces:
                interface.getExchangeTags()
                interface.partnerInterface.getExchangeTags()
                interface.getPostProcessingDataTags()
#

innerFluid = OpenFOAMParticipant("Inner-Fluid")
innerFluidInterface = innerFluid.addInterface()

outerFluid = OpenFOAMParticipant("Outer-Fluid")
outerFluidInterface = outerFluid.addInterface()

solid = CodeAsterParticipant("Solid")
outerSolidInterface = solid.addInterface()
innerSolidInterface = solid.addInterface()

outerSolidInterface.setPartnerInterface(outerFluidInterface)
innerSolidInterface.setPartnerInterface(innerFluidInterface)

participants = [innerFluid, outerFluid, solid]

for participant in participants:
    participant.getDataTags()

for participant in participants:
    participant.getMeshTags()

for participant in participants:
    participant.getParticipantTag()

# Coupling graph
couplings = [[solid, innerFluid], [outerFluid, solid]]
graph = nx.Graph()
[graph.add_edge(pair[0], pair[1]) for pair in couplings]
colors =  nx.coloring.greedy_color(graph, strategy=nx.coloring.strategy_largest_first)
cycles = len(nx.cycle_basis(graph)) > 0

## Input
steadyState = False
defaultExplicit = False
defaultParallel = True

## ===================
## Rules
## ===================

# Use explicit in steady state simulations or if specified by user
explicit = steadyState or defaultExplicit
implicit = not explicit

# Use parallel or multi if there are cyclic coupling dependecies
parallelORmulti = (cycles or defaultParallel)
# Use multi for parallel implicit coupling between more than two participants
multi = parallelORmulti and implicit and len(participants) > 2
# Use parallel coupling if there are two participants or if explicit coupling is used
parallel = parallelORmulti and not multi
# If not parallel nor multi, then use serial (= no cycles and default is serial)
serial = not parallel and not multi

timestep = 0.01
maxTime = 1.0
maxIterations = 30

# If multi, all couplings are treated together
if multi:

    couplingScheme = MultiCouplingScheme(timestep, maxTime, couplings)
    couplingScheme.getCouplingParticipantTags()
    couplingScheme.getExchangeTags()


# If not multi, couplings are treated per pair
else:

    for participantsPair in couplings:

        # Determine first and second participant
        if colors[participantsPair[0]] == 1:
            participantsPair.reverse()

        if implicit:
            couplingScheme = ImplicitCouplingScheme(timestep, maxTime, participantsPair, serial=serial)
        else:
            couplingScheme = CouplingScheme(timestep, maxTime, participantsPair, serial=serial)

        couplingScheme.getCouplingSchemeTag()
        couplingScheme.getCouplingParticipantTags()
        couplingScheme.getExchangeTags()
