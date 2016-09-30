import sys
from lxml import etree
from interface import *

class ParticipantFactory:
    def getParticipant(type, name):
        if type == "OpenFOAM":
            return OpenFOAMParticipant(name)
        elif type == "Code_Aster":
            return CodeAsterParticipant(name)
        elif type == "CalculiX":
            return CalculiXParticipant(name)
        else:
            print "Participant of type", type, "is not implemented."
            sys.exit(1)

    getParticipant=staticmethod(getParticipant)

class Participant(object):

    def __init__(self, name):
        self.name = name
        self.interfaces = []
        self.dataNameT = "Sink-Temperature-" + self.name
        self.dataNameHTC = "Heat-Transfer-Coefficient-" + self.name

    def addDataTagsTo(self, parent):
        etree.SubElement(parent, etree.QName("data", "scalar"), name=self.dataNameHTC)
        etree.SubElement(parent, etree.QName("data", "scalar"), name=self.dataNameT)

    def addMeshTagsTo(self, parent):
        for interface in self.interfaces:
            interface.addMeshTagsTo(parent)

    def addParticipantTagTo(self, parent):
        participant = etree.SubElement(parent, "participant", name=self.name)
        for interface in self.interfaces:
            interface.addProvideMeshTagsTo(participant)
            interface.partnerInterface.addFromMeshTagTo(participant)
            interface.addReadWriteMappingTagsTo(participant)

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


class CalculiXParticipant(Participant):

    def __init__(self, name):
        super(CalculiXParticipant, self).__init__(name)
        self.solverType = "CalculiX"

    def addInterface(self):
        interface = CalculiXInterface(self)
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
