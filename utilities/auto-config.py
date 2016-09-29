from lxml import etree
import matplotlib.pyplot as plt
import networkx as nx

class XMLNamespaces:
    data = "data"
    mapping = "mapping"
    couplingScheme = "coupling-scheme"
    postProcessing = "post-processing"

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

    def addProvideMeshTags(self, parent):
        etree.SubElement(parent, "use-mesh", name=self.readMesh, provide="yes")
        if self.readMesh != self.writeMesh:
            etree.SubElement(parent, "use-mesh", name=self.writeMesh, provide="yes")

    def addFromMeshTag(self, parent):
        e = etree.SubElement(parent, "use-mesh", name=self.writeMesh)
        e.set("from", self.participant.name)

    def addReadWriteMappingTags(self, parent):
        etree.SubElement(parent, "write-data", name=self.participant.dataNameT, mesh=self.writeMesh)
        etree.SubElement(parent, "write-data", name=self.participant.dataNameHTC, mesh=self.writeMesh)
        etree.SubElement(parent, "read-data", name=self.participant.dataNameT, mesh=self.readMesh)
        etree.SubElement(parent, "read-data", name=self.participant.dataNameHTC, mesh=self.readMesh)
        e = etree.SubElement(parent, etree.QName(XMLNamespaces.mapping, "nearest-neighbor"), direction="read", to=self.readMesh)
        e.set("from", self.partnerInterface.writeMesh)

    def addExchangeTags(self, parent):
        # From me to partner
        e = etree.SubElement(parent, "exchange", data=self.participant.dataNameT, mesh=self.writeMesh, to=self.partnerInterface.participant.name)
        e.set("from", self.participant.name)
        e = etree.SubElement(parent, "exchange", data=self.participant.dataNameHTC, mesh=self.writeMesh, to=self.partnerInterface.participant.name)
        e.set("from", self.participant.name)

    def addPostProcessingDataTags(self, parent):
        pass

class OpenFOAMInterface(Interface):

    def __init__(self, participant):
        super(OpenFOAMInterface, self).__init__(participant)

    def addMeshTags(self, parent):
        mesh = etree.SubElement(parent, "mesh", name=self.mesh)
        etree.SubElement(mesh, "use-data", name=self.participant.dataNameT)
        etree.SubElement(mesh, "use-data", name=self.participant.dataNameHTC)
        etree.SubElement(mesh, "use-data", name=self.partnerInterface.participant.dataNameT)
        etree.SubElement(mesh, "use-data", name=self.partnerInterface.participant.dataNameHTC)

    def setMeshNames(self):
        self.mesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Faces-Mesh"
        self.readMesh = self.mesh
        self.writeMesh = self.mesh

class CodeAsterInterface(Interface):

    def __init__(self, participant):
        super(CodeAsterInterface, self).__init__(participant)

    def addMeshTags(self, parent):
        writeMesh = etree.SubElement(parent, "mesh", name=self.writeMesh)
        etree.SubElement(writeMesh, "use-data", name=self.participant.dataNameT)
        etree.SubElement(writeMesh, "use-data", name=self.participant.dataNameHTC)
        readMesh = etree.SubElement(parent, "mesh", name=self.readMesh)
        etree.SubElement(readMesh, "use-data", name=self.partnerInterface.participant.dataNameT)
        etree.SubElement(readMesh, "use-data", name=self.partnerInterface.participant.dataNameHTC)

    def setMeshNames(self):
        self.readMesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Faces-Mesh"
        self.writeMesh = self.participant.name + "-to-" + self.partnerInterface.participant.name + "-Nodes-Mesh"

class Participant(object):

    def __init__(self, name):
        self.name = name
        self.interfaces = []
        self.dataNameT = "Sink-Temperature-" + self.name
        self.dataNameHTC = "Heat-Transfer-Coefficient-" + self.name

    def addDataTags(self, parent):
        HTC = etree.SubElement(parent, etree.QName(XMLNamespaces.data, "scalar"), name=self.dataNameHTC)

    def addMeshTags(self, parent):
        for interface in self.interfaces:
            interface.addMeshTags(parent)

    def addParticipantTag(self, parent):
        participant = etree.SubElement(parent, "participant", name=self.name)
        for interface in self.interfaces:
            interface.addProvideMeshTags(participant)
            interface.partnerInterface.addFromMeshTag(participant)
            interface.addReadWriteMappingTags(participant)

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
        self.schemeName = scheme + "-" + self.type

    def addCouplingSchemeTag(self, parent):
        couplingSchemeTag = etree.SubElement(parent, etree.QName(XMLNamespaces.couplingScheme, self.schemeName))
        self.addTimestepTag(couplingSchemeTag)
        self.addMaxTimeTag(couplingSchemeTag)
        self.addCouplingParticipantTags(couplingSchemeTag)
        self.addExchangeTags(couplingSchemeTag)
        return couplingSchemeTag

    def addTimestepTag(self, parent):
        etree.SubElement(parent, "timestep-length", value=str(self.timestep))

    def addMaxTimeTag(self, parent):
        etree.SubElement(parent, "max-time", value=str(self.maxTime))

    def addCouplingParticipantTags(self, parent):
        etree.SubElement(parent, "participants", first=self.participants[0].name, second=self.participants[1].name)

    def addExchangeTags(self, parent):
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            interface.addExchangeTags(parent)
            interface.partnerInterface.addExchangeTags(parent)
            interface.addPostProcessingDataTags(parent)


class ImplicitCouplingScheme(CouplingScheme):
    def __init__(self, timestep, maxTime, participants, maxIterations=50, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
        self.schemeName = "implicit"
        self.maxIterations = maxIterations
    def addCouplingSchemeTag(self, parent):
        couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTag(parent)
        self.addMaxIterationsTag(couplingSchemeTag)
        ppTag = self.addPostProcessingTag(couplingSchemeTag)
    def getRelativeConvergenceMeasureTags(self):
        pass
    def addMaxIterationsTag(self, parent):
        etree.SubElement(parent, "max-iterations", value=str(self.maxIterations))
    def addPostProcessingTag(self, parent):
        return etree.SubElement(parent, etree.QName(XMLNamespaces.postProcessing, "IQN-ILS"))
    def addPostProcessingDataTags(self, participant):
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
        self.schemeName="multi"

    def addCouplingParticipantTags(self, parent):
        for participant in self.participants:
            etree.SubElement(parent, "participant", name=participant.name)

    def addExchangeTags(self, parent):
        for pair in self.participantPairs:
            interfaces = pair[0].getInterfacesWith(pair[1])
            for interface in interfaces:
                interface.addExchangeTags(parent)
                interface.partnerInterface.addExchangeTags(parent)
                interface.addPostProcessingDataTags(parent)
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


nsmap = {
    'data': XMLNamespaces.data,
    'mapping': XMLNamespaces.mapping,
    'coupling-scheme': XMLNamespaces.couplingScheme,
    'post-processing': XMLNamespaces.postProcessing
}
preciceConfigurationTag = etree.Element("precice-configuration", nsmap=nsmap)

participants = [innerFluid, solid, outerFluid]

for participant in participants:
    participant.addDataTags(preciceConfigurationTag)

for participant in participants:
    participant.addMeshTags(preciceConfigurationTag)

for participant in participants:
    participant.addParticipantTag(preciceConfigurationTag)


# Coupling graph
couplings = [[solid, innerFluid], [solid, outerFluid]]
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

multi = False


# If multi, all couplings are treated together
if multi:
    couplingScheme = MultiCouplingScheme(timestep, maxTime, couplings)
    couplingScheme.addCouplingSchemeTag(preciceConfigurationTag)

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
        couplingScheme.addCouplingSchemeTag(preciceConfigurationTag)

print etree.tostring(preciceConfigurationTag, pretty_print=True)
