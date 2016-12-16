from lxml import etree


class Interface(object):

    def __init__(self, participant, name=None):
        self.participant = participant
        self.name = name
        self.mesh = None
        self.write_mesh = None
        self.read_mesh = None
        self.partner_interface = None

    def set_partner_interface(self, partner_interface):
        self.partner_interface = partner_interface
        if self.partner_interface.partner_interface is None:
            self.partner_interface.set_partner_interface(self)
        self.set_mesh_names()

    def set_mesh_names(self):
        # By default, use the same read and write mesh
        if self.name:
            self.mesh = self.name
        else:
            self.mesh = self.participant.name + "-to-" + self.partner_interface.participant.name
        self.read_mesh = self.mesh
        self.write_mesh = self.mesh

    def add_mesh_tags_to(self, parent):
        mesh = etree.SubElement(parent, "mesh", name=self.mesh)
        for write_data in self.write_data:
            etree.SubElement(mesh, "use-data", name=write_data)
        for read_data in self.partner_interface.write_data:
            etree.SubElement(mesh, "use-data", name=read_data)

    def add_provide_mesh_tags_to(self, parent):
        etree.SubElement(parent, "use-mesh", name=self.read_mesh, provide="yes")
        if self.read_mesh != self.write_mesh:
            etree.SubElement(parent, "use-mesh", name=self.write_mesh, provide="yes")

    def add_from_mesh_tags_to(self, parent):
        e = etree.SubElement(parent, "use-mesh", name=self.write_mesh, decomposition="pre-filter-post-filter")
        e.set("from", self.participant.name)

    def add_read_write_mapping_tags_to(self, parent):
        for write_data in self.write_data:
            etree.SubElement(parent, "write-data", name=write_data, mesh=self.write_mesh)
        for read_data in self.partner_interface.write_data:
            etree.SubElement(parent, "read-data", name=read_data, mesh=self.read_mesh)
        e = etree.SubElement(parent, etree.QName("mapping", "nearest-neighbor"),
                             direction="read", to=self.read_mesh, constraint="consistent")
        e.set("from", self.partner_interface.write_mesh)

    def add_exchange_tags_to(self, parent, initialize=False):
        # From me to partner
        for write_data in self.write_data:
            e = etree.SubElement(parent, "exchange", data=write_data, mesh=self.write_mesh)
            e.set("from", self.participant.name)
            e.set("to", self.partner_interface.participant.name)
            if initialize:
                e.set("initialize", "yes")


class OpenFOAMInterface(Interface):

    def __init__(self, participant, name=None):
        super(OpenFOAMInterface, self).__init__(participant, name)
        self.write_data = ["Sink-Temperature-" + self.participant.name,
                           "Heat-Transfer-Coefficient-" + self.participant.name]


class CalculiXInterface(Interface):

    def __init__(self, participant, name=None):
        super(CalculiXInterface, self).__init__(participant, name)
        self.write_data = ["Sink-Temperature-" + self.participant.name,
                           "Heat-Transfer-Coefficient-" + self.participant.name]


class CodeAsterInterface(Interface):

    def __init__(self, participant, name=None):
        super(CodeAsterInterface, self).__init__(participant, name)
        self.write_data = ["Sink-Temperature-" + self.participant.name,
                           "Heat-Transfer-Coefficient-" + self.participant.name]

    def add_mesh_tags_to(self, parent):
        write_mesh = etree.SubElement(parent, "mesh", name=self.write_mesh)
        for write_data in self.write_data:
            etree.SubElement(write_mesh, "use-data", name=write_data)
        read_mesh = etree.SubElement(parent, "mesh", name=self.read_mesh)
        for read_data in self.partner_interface.write_data:
            etree.SubElement(read_mesh, "use-data", name=read_data)

    def set_mesh_names(self):
        # For Robin-Robin coupling, Code_Aster has its read-data at the faces
        # and its write-data at the nodes
        if self.name:
            self.mesh = self.name
        else:
            self.mesh = self.participant.name + "-to-" + self.partner_interface.participant.name
        self.read_mesh = self.mesh + "-Faces"
        self.write_mesh = self.mesh + "-Nodes"
