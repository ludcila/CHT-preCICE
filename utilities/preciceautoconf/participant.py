from lxml import etree
from interface import *
from xmlns import *

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
