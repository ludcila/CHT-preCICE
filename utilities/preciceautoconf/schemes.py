from lxml import etree


class CouplingScheme(object):
    
    def __init__(self, coupling_config, participants):
        self.coupling_config = coupling_config
        self.time_step = coupling_config.time_step
        self.max_time = coupling_config.max_time
        coupling_config.sort_participants(participants)
        self.participants = participants
        self.is_serial = coupling_config.is_serial()
        if self.is_serial:
            self.serialOrParallel = "serial"
        else:
            self.serialOrParallel = "parallel"
        self.explicit_or_implicit = "explicit" # Base class is explicit

    def get_coupling_scheme_name(self):
        return self.serialOrParallel + "-" + self.explicit_or_implicit

    def add_coupling_scheme_tag_to(self, parent):
        coupling_scheme_tag = etree.SubElement(parent, etree.QName("coupling-scheme", self.get_coupling_scheme_name()))
        self.add_time_step_tag_to(coupling_scheme_tag)
        self.add_max_time_tag_to(coupling_scheme_tag)
        self.add_coupling_participant_tags_to(coupling_scheme_tag)
        self.add_exchange_tags_to(coupling_scheme_tag)
        return coupling_scheme_tag

    def add_time_step_tag_to(self, parent):
        etree.SubElement(parent, "time_step-length", value=str(self.time_step))

    def add_max_time_tag_to(self, parent):
        etree.SubElement(parent, "max-time", value=str(self.max_time))

    def add_coupling_participant_tags_to(self, parent):
        etree.SubElement(parent, "participants", first=self.participants[0].name, second=self.participants[1].name)

    def add_exchange_tags_to(self, parent):
        interfaces = self.participants[0].get_interfaces_with(self.participants[1])
        for interface in interfaces:
            # First participant initializes the coupling data of the second participant only in a parallel scheme
            interface.add_exchange_tags_to(parent, initialize=not self.is_serial)
            # Second participant initializes the coupling data of the first participant
            interface.partner_interface.add_exchange_tags_to(parent, initialize=True)
            interface.add_post_processing_data_tags_to(parent)

    def add_m2n_tag_to(self, parent, type="sockets"):
        e = etree.SubElement(parent, etree.QName("m2n", type), to=self.participants[1].name)
        e.set("from", self.participants[0].name)
        if self.participants[0].domain_decomposed or self.participants[1].domain_decomposed:
            e.set("distribution-type", "scatter-gather")


class ImplicitCouplingScheme(CouplingScheme):

    def __init__(self, coupling_config, participants):
        super(ImplicitCouplingScheme, self).__init__(coupling_config, participants)
        self.explicit_or_implicit = "implicit"
        self.max_iterations = coupling_config.max_iterations

    def add_coupling_scheme_tag_to(self, parent):
        # If there are multiple interfaces between the two participants,
        # we generate a coupling-scheme tag for each interface,
        # otherwise we will be able to apply post-processing only to one data
        interfaces = self.participants[0].get_interfaces_with(self.participants[1])
        for interface in interfaces:
            coupling_scheme_tag = super(ImplicitCouplingScheme, self).add_coupling_scheme_tag_to(parent)
            interface.add_exchange_tags_to(coupling_scheme_tag)
            interface.partner_interface.add_exchange_tags_to(coupling_scheme_tag)
            self.addmax_iterationsTagTo(coupling_scheme_tag)
            self.add_post_processing_tag_to(coupling_scheme_tag)

    def add_exchange_tags_to(self, parent):
        # Override add_exchange_tags_to from CouplingScheme which puts the exchange tags
        # from all interface pairs under the same coupling-scheme tag
        # For implicit coupling we will have a separate coupling-scheme tag for
        # each interface pair, so that post-processing can be applied to all the data
        pass

    def get_relative_convergence_measures(self):
        pass

    def addmax_iterationsTagTo(self, parent):
        print self.max_iterations
        etree.SubElement(parent, "max-iterations", value=str(self.max_iterations))

    def add_post_processing_tag_to(self, parent):
        post_processing = IQNILSPostProcessing()
        post_processing.add_post_processing_tag_to(parent)

    def add_post_processing_data_tags_to(self, parent):
        if self.is_serial:
            pass
            # Apply post-processing only to the data from the second participant
            # etree.SubElement(parent, "data", name=self.participants[1].dataNameT, mesh=self.participants[1].writeMesh) # check
        else:
            pass
            # Apply post-processing to data from both participants
            # etree.SubElement(parent, "data", name=self.participants[0].dataNameT, mesh=self.participants[0].writeMesh) # check
            # etree.SubElement(parent, "data", name=self.participants[1].dataNameT, mesh=self.participants[1].writeMesh) # check


class MultiCouplingScheme(ImplicitCouplingScheme):

    def __init__(self, coupling_config, participant_pairs):
        self.participant_pairs = participant_pairs
        self.participants = []
        for pair in self.participant_pairs:
            self.participants.append(pair[0])
            self.participants.append(pair[1])
        self.participants = list(set(self.participants))
        super(MultiCouplingScheme, self).__init__(coupling_config, self.participants)
        self.scheme_name = "multi"

    def get_coupling_scheme_name(self):
        return "multi"

    def add_coupling_scheme_tag_to(self, parent):
        coupling_scheme_tag = super(ImplicitCouplingScheme, self).add_coupling_scheme_tag_to(parent)
        self.addmax_iterationsTagTo(coupling_scheme_tag)
        self.add_post_processing_tag_to(coupling_scheme_tag)

    def add_coupling_participant_tags_to(self, parent):
        for participant in self.participants:
            etree.SubElement(parent, "participant", name=participant.name)

    def add_exchange_tags_to(self, parent):
        for participants in self.participant_pairs:
            interfaces = participants[0].get_interfaces_with(participants[1])
            for interface in interfaces:
                interface.add_exchange_tags_to(parent, initialize=True)
                interface.partner_interface.add_exchange_tags_to(parent, initialize=True)

    def add_m2n_tag_to(self, parent, type="sockets"):
        for participants in self.participant_pairs:
            e = etree.SubElement(parent, etree.QName("m2n", type), to=participants[1].name)
            e.set("from", participants[0].name)
            if participants[0].domain_decomposed or participants[1].domain_decomposed:
                e.set("distribution-type", "scatter-gather")


class IQNILSPostProcessing:

    def __init__(self, initial_relaxation=1.0, max_used_iterations=100):
        self.initial_relaxation = initial_relaxation
        self.max_used_iterations = max_used_iterations

    def add_post_processing_tag_to(self, parent):
        e = etree.SubElement(parent, etree.QName("post-processing", "IQN-ILS"))
        self.add_initial_relaxation_tag_to(e)
        self.add_max_used_iterations_tag(e)
        self.add_filter_tag_to(e)
        self.add_preconditioner_tag_to(e)
        return e

    def add_max_used_iterations_tag(self, parent):
        etree.SubElement(parent, "max-used-iterations", value=str(self.max_used_iterations))

    def add_initial_relaxation_tag_to(self, parent):
        etree.SubElement(parent, "initial-relaxation", value=str(self.initial_relaxation))

    def add_preconditioner_tag_to(self, parent):
        etree.SubElement(parent, "preconditioner", type="residual-sum")

    def add_filter_tag_to(self, parent):
        etree.SubElement(parent, "filter", type="QR1", limit=str(1e-6))
