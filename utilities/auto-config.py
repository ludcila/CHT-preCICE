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

    def provideMeshes(self):
        print '\t<use-mesh name="' + self.readMesh + '" provide="yes"/>'
        if self.readMesh != self.writeMesh:
            print '\t<use-mesh name="' + self.writeMesh + '" provide="yes"/>'

    def fromMeshes(self):
        print '\t<use-mesh name="' + self.writeMesh + '" from="' + self.participant.name + '"/>'

    def addReadWriteMappingTags(self):
        print '\t<write-data name="' + self.participant.dataNameT + '" mesh="' + self.writeMesh + '"/>'
        print '\t<write-data name="' + self.participant.dataNameHTC + '" mesh="' + self.writeMesh + '"/>'
        print '\t<read-data name="' + self.partnerInterface.participant.dataNameT + '" mesh="' + self.readMesh + '"/>'
        print '\t<read-data name="' + self.partnerInterface.participant.dataNameHTC + '" mesh="' + self.readMesh + '"/>'
        print '\t<mapping:nearest-neighbor direction="read" from="' + self.partnerInterface.writeMesh + '" to="' + self.readMesh + '"/>'

    def addExchangeTags(self):
        # From me to partner
        print '\t<exchange data="' + self.participant.dataNameT + '" mesh="' + self.writeMesh + '" from="' + self.participant.name + '" to="' + self.partnerInterface.participant.name + '"/>'
        print '\t<exchange data="' + self.participant.dataNameHTC + '" mesh="' + self.writeMesh + '" from="' + self.participant.name + '" to="' + self.partnerInterface.participant.name + '"/>'

    def addPostProcessingData(self):
        pass

class OpenFOAMInterface(Interface):

    def __init__(self, participant):
        super(OpenFOAMInterface, self).__init__(participant)

    def addMeshTags(self):
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

    def addMeshTags(self):
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

    def addDataTag(self):
        print '<data:scalar name="' + self.dataNameHTC + '"/>'
        print '<data:scalar name="' + self.dataNameT + '"/>'

    def addMeshTags(self):
        for interface in self.interfaces:
            interface.addMeshTags()

    def addParticipantTag(self):
        print '<participant name="' + self.name + '"/>'
        for interface in self.interfaces:
            interface.provideMeshes()
            interface.partnerInterface.fromMeshes()
            interface.addReadWriteMappingTags()
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
        self.type = "explicit"
        self.scheme = scheme + "-" + self.type

    def addCouplingScheme(self):
        print "<coupling-scheme:" + self.scheme + ">"

    def addParticipants(self):
        print '<participants first="' + self.participants[0].name + '"' ' second="' + self.participants[1].name + '"/>'

    def addExchangeTags(self):
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            interface.addExchangeTags()
            interface.partnerInterface.addExchangeTags()
            interface.addPostProcessingData()


class ImplicitCouplingScheme(CouplingScheme):
    def __init__(self, timestep, maxTime, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(self, timestep, maxTime, participants, serial=False)
    def addRelativeConvergenceMeasure(self):
        pass
    def addMaxIterations(self):
        pass
    def addPostProcessingDataForParticipant(self, participant):
        pass

class MultiCouplingScheme(CouplingScheme):
    def addParticipants(self):
        if self.type == "implicit":
            for participant in participants:
                print '<participant name="' + participant.name + '"/>'
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
    participant.addDataTag()

for participant in participants:
    participant.addMeshTags()

for participant in participants:
    participant.addParticipantTag()

# Coupling graph
couplings = [[solid, innerFluid], [outerFluid, solid]]
graph = nx.Graph()
[graph.add_edge(pair[0], pair[1]) for pair in couplings]
colors =  nx.coloring.greedy_color(graph, strategy=nx.coloring.strategy_largest_first)
cycles = len(nx.cycle_basis(graph)) > 0

## Input
steadyState = True
defaultExplicit = True
defaultParallel = True

explicit = steadyState or defaultExplicit
parallel = cycles or defaultParallel

timestep = 0.01
maxTime = 1.0
maxIterations = 30

for coupling in couplings:
    if colors[coupling[0]] == 1:
        coupling.reverse()
    couplingScheme = CouplingScheme(timestep, maxTime, coupling)
    couplingScheme.addParticipants()
    couplingScheme.addExchangeTags()
