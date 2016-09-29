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
        self.serial = serial
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
    def __init__(self, timestep, maxTime, maxIterations, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
        self.schemeName = "implicit"
        self.maxIterations = maxIterations
    def addCouplingSchemeTag(self, parent):
        couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTag(parent)
        self.addMaxIterationsTag(couplingSchemeTag)
        postProcessingTag = self.addPostProcessingTag(couplingSchemeTag)
        self.addPostProcessingDataTags(postProcessingTag)
    def getRelativeConvergenceMeasureTags(self):
        pass
    def addMaxIterationsTag(self, parent):
        print self.maxIterations
        etree.SubElement(parent, "max-iterations", value=str(self.maxIterations))
    def addPostProcessingTag(self, parent):
        return etree.SubElement(parent, etree.QName(XMLNamespaces.postProcessing, "IQN-ILS"))
    def addPostProcessingDataTags(self, parent):
        if self.serial:
            pass
            # Apply post-processing only to the data from the second participant
            # etree.SubElement(parent, "data", name=self.participants[1].dataNameT, mesh=self.participants[1].writeMesh) # check
        else:
            pass
            # Apply post-processing to data from both participants
            # etree.SubElement(parent, "data", name=self.participants[0].dataNameT, mesh=self.participants[0].writeMesh) # check
            # etree.SubElement(parent, "data", name=self.participants[1].dataNameT, mesh=self.participants[1].writeMesh) # check

class MultiCouplingScheme(ImplicitCouplingScheme):

    def __init__(self, timestep, maxTime, maxIterations, participantPairs, serial=False):
        self.participantPairs = participantPairs
        self.participants = []
        for pair in self.participantPairs:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))
        super(MultiCouplingScheme, self).__init__(timestep, maxTime, maxIterations, self.participants, False)
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

class CouplingConfiguration:

    def __init__(self, couplings, steadyState, forceExplicit, forceParallel):
        self.steadyState = steadyState
        self.couplings = couplings
        self.forceExplicit = forceExplicit
        self.forceParallel = forceParallel
        self.createGraph()
        self.getParticipantsFromCoupling()

    def createGraph(self):
        self.graph = nx.Graph()
        [self.graph.add_edge(pair[0], pair[1]) for pair in self.couplings]
        self.colors =  nx.coloring.greedy_color(self.graph, strategy=nx.coloring.strategy_largest_first)

    def hasCycles(self):
        return len(nx.cycle_basis(self.graph)) > 0

    def getColoring(self):
        return self.colors

    def getParticipantsFromCoupling(self):
        self.participants = []
        for pair in couplings:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))

    # Use explicit in steady state simulations or if specified by user
    def explicit(self):
        return self.steadyState or self.forceExplicit

    # Use implicit if not using explicit
    def implicit(self):
        return not self.explicit()

    # Use parallel or multi if there are cyclic coupling dependecies
    def parallelOrMulti(self):
        return self.hasCycles() or self.forceParallel

    # Use multi for parallel implicit coupling between more than two participants
    def multi(self):
        return self.parallelOrMulti() and self.implicit() and len(self.participants) > 2

    # Use parallel, for parallel coupling between two participants, or explicit coupling between more than two
    def parallel(self):
        return self.parallelOrMulti() and not self.multi()

    # Use serial if not using multi or parallel
    def serial(self):
        return not self.multi() and not self.parallel()





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

config = CouplingConfiguration(
    couplings=couplings,
    steadyState=False,
    forceExplicit=False,
    forceParallel=True
)

timestep = 0.01
maxTime = 1.0
maxIterations = 30


# If multi, all couplings are treated together
if config.multi():
    couplingScheme = MultiCouplingScheme(timestep, maxTime, maxIterations, couplings)
    couplingScheme.addCouplingSchemeTag(preciceConfigurationTag)

# If not multi, couplings are treated per pair
else:
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
