from lxml import etree

class Interface(object):

    def __init__(self, participant, name=None):
        self.participant = participant
        self.name = name

    def setPartnerInterface(self, partnerInterface):
        self.partnerInterface = partnerInterface
        if not hasattr(partnerInterface, "partnerInterface"):
            partnerInterface.setPartnerInterface(self)
        self.setMeshNames()

    def setMeshNames(self):
        # By default, use the same read and write mesh
        if self.name:
            self.mesh = self.name
        else:
            self.mesh = self.participant.name + "-to-" + self.partnerInterface.participant.name
        self.readMesh = self.mesh
        self.writeMesh = self.mesh

    def addMeshTagsTo(self, parent):
        mesh = etree.SubElement(parent, "mesh", name=self.mesh)
        etree.SubElement(mesh, "use-data", name=self.participant.dataNameT)
        etree.SubElement(mesh, "use-data", name=self.participant.dataNameHTC)
        etree.SubElement(mesh, "use-data", name=self.partnerInterface.participant.dataNameT)
        etree.SubElement(mesh, "use-data", name=self.partnerInterface.participant.dataNameHTC)

    def addProvideMeshTagsTo(self, parent):
        etree.SubElement(parent, "use-mesh", name=self.readMesh, provide="yes")
        if self.readMesh != self.writeMesh:
            etree.SubElement(parent, "use-mesh", name=self.writeMesh, provide="yes")

    def addFromMeshTagTo(self, parent):
        e = etree.SubElement(parent, "use-mesh", name=self.writeMesh)
        e.set("from", self.participant.name)

    def addReadWriteMappingTagsTo(self, parent):
        etree.SubElement(parent, "write-data", name=self.participant.dataNameT, mesh=self.writeMesh)
        etree.SubElement(parent, "write-data", name=self.participant.dataNameHTC, mesh=self.writeMesh)
        etree.SubElement(parent, "read-data", name=self.participant.dataNameT, mesh=self.readMesh)
        etree.SubElement(parent, "read-data", name=self.participant.dataNameHTC, mesh=self.readMesh)
        e = etree.SubElement(parent, etree.QName("mapping", "nearest-neighbor"), direction="read", to=self.readMesh)
        e.set("from", self.partnerInterface.writeMesh)

    def addExchangeTagsTo(self, parent, initialize=False):
        # From me to partner
        e = etree.SubElement(parent, "exchange", data=self.participant.dataNameT, mesh=self.writeMesh, to=self.partnerInterface.participant.name)
        e.set("from", self.participant.name)
        if initialize:
            e.set("initialize", "yes")
        e = etree.SubElement(parent, "exchange", data=self.participant.dataNameHTC, mesh=self.writeMesh, to=self.partnerInterface.participant.name)
        e.set("from", self.participant.name)
        if initialize:
            e.set("initialize", "yes")

    def addPostProcessingDataTagsTo(self, parent):
        pass

class OpenFOAMInterface(Interface):

    def __init__(self, participant, name=None):
        super(OpenFOAMInterface, self).__init__(participant, name)

class CalculiXInterface(Interface):

    def __init__(self, participant, name=None):
        super(CalculiXInterface, self).__init__(participant, name)

class CodeAsterInterface(Interface):

    def __init__(self, participant, name=None):
        super(CodeAsterInterface, self).__init__(participant, name)

    def addMeshTagsTo(self, parent):
        writeMesh = etree.SubElement(parent, "mesh", name=self.writeMesh)
        etree.SubElement(writeMesh, "use-data", name=self.participant.dataNameT)
        etree.SubElement(writeMesh, "use-data", name=self.participant.dataNameHTC)
        readMesh = etree.SubElement(parent, "mesh", name=self.readMesh)
        etree.SubElement(readMesh, "use-data", name=self.partnerInterface.participant.dataNameT)
        etree.SubElement(readMesh, "use-data", name=self.partnerInterface.participant.dataNameHTC)

    def setMeshNames(self):
        # For Robin-Robin coupling, Code_Aster has its read-data at the faces
        # and its write-data at the nodes
        if self.name:
            self.mesh = self.name
        else:
            self.mesh = self.participant.name + "-to-" + self.partnerInterface.participant.name
        self.readMesh = self.mesh + "-Faces"
        self.writeMesh = self.mesh + "-Nodes"
