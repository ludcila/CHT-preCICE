import logging
import sys
from interface import *


class ParticipantFactory:

    def get_participant(name, data):
        if data["solver"] == "OpenFOAM":
            return OpenFOAMParticipant(name, data)
        elif data["solver"] == "Code_Aster":
            return CodeAsterParticipant(name, data)
        elif data["solver"] == "CalculiX":
            return CalculiXParticipant(name, data)
        else:
            print "Participant of type", data["solver"], "is not implemented."
            sys.exit(1)

    get_participant = staticmethod(get_participant)


class Participant(object):

    def __init__(self, name, data):
        self.name = name
        self.populate_data(data)
        self.interfaces = []
        self.domain_decomposed = None
        self.data_name_T = "Sink-Temperature-" + self.name
        self.data_name_HTC = "Heat-Transfer-Coefficient-" + self.name

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

    def populate_data(self, data):
        # Check and populate basic data that all participants should have
        try:
            self.domain_decomposed = data["domain-decomposed"]
            data["interfaces"]
            data["directory"]
        except Exception as e:
            logging.error(str(e) + " attribute not provided for participant " + self.name)
            sys.exit(1)


class OpenFOAMParticipant(Participant):

    def __init__(self, name, data):
        super(OpenFOAMParticipant, self).__init__(name, data)
        self.solver_type = "OpenFOAM"

    def add_interface(self, name=None):
        interface = OpenFOAMInterface(self, name)
        self.interfaces.append(interface)
        return interface


class CalculiXParticipant(Participant):

    def __init__(self, name, data):
        super(CalculiXParticipant, self).__init__(name, data)
        self.solver_type = "CalculiX"

    def add_interface(self, name=None):
        interface = CalculiXInterface(self, name)
        self.interfaces.append(interface)
        return interface


class CodeAsterParticipant(Participant):

    def __init__(self, name, data):
        super(CodeAsterParticipant, self).__init__(name, data)
        self.solver_type = "Code_Aster"

    def add_interface(self, name=None):
        interface = CodeAsterInterface(self, name)
        self.interfaces.append(interface)
        return interface

    def populate_data(self, data):

        try:
            data["non-linear"]
        except Exception as e:
            logging.error(str(e) + " attribute not provided for Code_Aster participant " + self.name)
            sys.exit(1)

        for interface in data["interfaces"]:
            try:
                    interface["material-id"]
            except Exception as e:
                logging.error(str(e) + " attribute not provided for an interface of Code_Aster participant " + self.name)
                sys.exit(1)
