from lxml import etree

class CouplingScheme(object):
    def __init__(self, timestep, maxTime, participants, serial=False):
        self.timestep = timestep
        self.maxTime = maxTime
        self.participants = participants
        self.serial = serial
        if serial:
            self.serialOrParallel = "serial"
        else:
            self.serialOrParallel = "parallel"
        self.explicitOrImplicit = "explicit" # Base class is explicit

    def getCouplingSchemeName(self):
        return self.serialOrParallel + "-" + self.explicitOrImplicit

    def addCouplingSchemeTagTo(self, parent):
        couplingSchemeTag = etree.SubElement(parent, etree.QName("coupling-scheme", self.getCouplingSchemeName()))
        self.addTimestepTagTo(couplingSchemeTag)
        self.addMaxTimeTagTo(couplingSchemeTag)
        self.addCouplingParticipantTagsTo(couplingSchemeTag)
        self.addExchangeTagsTo(couplingSchemeTag)
        return couplingSchemeTag

    def addTimestepTagTo(self, parent):
        etree.SubElement(parent, "timestep-length", value=str(self.timestep))

    def addMaxTimeTagTo(self, parent):
        etree.SubElement(parent, "max-time", value=str(self.maxTime))

    def addCouplingParticipantTagsTo(self, parent):
        etree.SubElement(parent, "participants", first=self.participants[0].name, second=self.participants[1].name)

    def addExchangeTagsTo(self, parent):
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            # First participant initializes the coupling data of the second participant only in a parallel scheme
            interface.addExchangeTagsTo(parent, initialize=not self.serial)
            # Second participant initializes the coupling data of the first participant
            interface.partnerInterface.addExchangeTagsTo(parent, initialize=True)
            interface.addPostProcessingDataTagsTo(parent)

    def addM2nTagTo(self, parent, type="sockets"):
        e = etree.SubElement(parent, etree.QName("m2n", type), to=self.participants[1].name)
        e.set("from", self.participants[0].name)
        if self.participants[0].domainDecomposed or self.participants[1].domainDecomposed:
            e.set("distribution-type", "scatter-gather")


class ImplicitCouplingScheme(CouplingScheme):

    def __init__(self, timestep, maxTime, maxIterations, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
        self.explicitOrImplicit = "implicit"
        self.maxIterations = maxIterations

    def addCouplingSchemeTagTo(self, parent):
        # If there are multiple interfaces between the two participants,
        # we generate a coupling-scheme tag for each interface,
        # otherwise we will be able to apply post-processing only to one data
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTagTo(parent)
            interface.addExchangeTagsTo(couplingSchemeTag)
            interface.partnerInterface.addExchangeTagsTo(couplingSchemeTag)
            self.addMaxIterationsTagTo(couplingSchemeTag)
            postProcessingTag = self.addPostProcessingTagTo(couplingSchemeTag)
            self.addPostProcessingDataTagsTo(postProcessingTag)

    def addExchangeTagsTo(self, parent):
        # Override addExchangeTagsTo from CouplingScheme which puts the exchange tags
        # from all interface pairs under the same coupling-scheme tag
        # For implicit coupling we will have a separate coupling-scheme tag for
        # each interface pair, so that post-processing can be applied to all the data
        pass

    def getRelativeConvergenceMeasureTagsTo(self):
        pass

    def addMaxIterationsTagTo(self, parent):
        print self.maxIterations
        etree.SubElement(parent, "max-iterations", value=str(self.maxIterations))

    def addPostProcessingTagTo(self, parent):
        return etree.SubElement(parent, etree.QName("post-processing", "IQN-ILS"))

    def addPostProcessingDataTagsTo(self, parent):
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

    def getCouplingSchemeName(self):
        return "multi"

    def addCouplingSchemeTagTo(self, parent):
        couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTagTo(parent)
        self.addMaxIterationsTagTo(couplingSchemeTag)
        postProcessingTag = self.addPostProcessingTagTo(couplingSchemeTag)
        self.addPostProcessingDataTagsTo(postProcessingTag)

    def addCouplingParticipantTagsTo(self, parent):
        for participant in self.participants:
            etree.SubElement(parent, "participant", name=participant.name)

    def addExchangeTagsTo(self, parent):
        for participants in self.participantPairs:
            interfaces = participants[0].getInterfacesWith(participants[1])
            for interface in interfaces:
                interface.addExchangeTagsTo(parent, initialize=True)
                interface.partnerInterface.addExchangeTagsTo(parent, initialize=True)
                interface.addPostProcessingDataTagsTo(parent)

    def addM2nTagTo(self, parent, type="sockets"):
        for participants in self.participantPairs:
            e = etree.SubElement(parent, etree.QName("m2n", type), to=participants[1].name)
            e.set("from", participants[0].name)
            if participants[0].domainDecomposed or participants[1].domainDecomposed:
                e.set("distribution-type", "scatter-gather")
