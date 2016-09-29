from lxml import etree
from xmlns import *

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
