import sys
from interface import *


class ParticipantFactory:

    def get_participant(type, name, domain_decomposed=False):
        if type == "OpenFOAM":
            return OpenFOAMParticipant(name, domain_decomposed)
        elif type == "Code_Aster":
            return CodeAsterParticipant(name)
        elif type == "CalculiX":
            return CalculiXParticipant(name)
        else:
            print "Participant of type", type, "is not implemented."
            sys.exit(1)

    get_participant = staticmethod(get_participant)


class Participant(object):

    def __init__(self, name, domain_decomposed=False):
        self.name = name
        self.interfaces = []
        self.data_name_T = "Sink-Temperature-" + self.name
        self.data_name_HTC = "Heat-Transfer-Coefficient-" + self.name
        self.domain_decomposed = domain_decomposed

    def add_data_tags_to(self, parent):
        etree.SubElement(parent, etree.QName("data", "scalar"), name=self.data_name_HTC)
        etree.SubElement(parent, etree.QName("data", "scalar"), name=self.data_name_T)

    def add_mesh_tags_to(self, parent):
        for interface in self.interfaces:
            interface.add_mesh_tags_to(parent)

    def add_participant_tag_to(self, parent):
        participant = etree.SubElement(parent, "participant", name=self.name)
        if self.domain_decomposed:
            etree.SubElement(participant, etree.QName("master", "mpi-single"))
        for interface in self.interfaces:
            interface.add_provide_mesh_tags_to(participant)
            interface.partner_interface.add_from_mesh_tags_to(participant)
            interface.add_read_write_mapping_tags_to(participant)

    def get_interfaces_with(self, partner):
        interfaces = []
        for interface in self.interfaces:
            if interface.partner_interface.participant == partner:
                interfaces.append(interface)
        return interfaces

    def has_interfaces_with(self, partner):
        return len(self.get_interfaces_with(partner)) > 0


class OpenFOAMParticipant(Participant):

    def __init__(self, name, domain_decomposed=False):
        super(OpenFOAMParticipant, self).__init__(name, domain_decomposed)
        self.solver_type = "OpenFOAM"

    def add_interface(self, name=None):
        interface = OpenFOAMInterface(self, name)
        self.interfaces.append(interface)
        return interface


class CalculiXParticipant(Participant):

    def __init__(self, name, domain_decomposed=False):
        super(CalculiXParticipant, self).__init__(name, domain_decomposed)
        self.solver_type = "CalculiX"

    def add_interface(self, name=None):
        interface = CalculiXInterface(self, name)
        self.interfaces.append(interface)
        return interface


class CodeAsterParticipant(Participant):

    def __init__(self, name, domain_decomposed=False):
        super(CodeAsterParticipant, self).__init__(name, domain_decomposed)
        self.solver_type = "CodeAster"

    def add_interface(self, name=None):
        interface = CodeAsterInterface(self, name)
        self.interfaces.append(interface)
        return interface
