from lxml import etree

class CouplingScheme(object):
    def __init__(self, timestep, maxTime, participants, serial=False):
        self.timestep = timestep
        self.maxTime = maxTime
        self.participants = participants
        self.serial = serial
        if serial:
            scheme = "serial"
        else:
            scheme = "parallel"
        self.type = "explicit"
        self.schemeName = scheme + "-" + self.type

    def addCouplingSchemeTagTo(self, parent):
        couplingSchemeTag = etree.SubElement(parent, etree.QName("coupling-scheme", self.schemeName))
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
            interface.addExchangeTagsTo(parent)
            interface.partnerInterface.addExchangeTagsTo(parent)
            interface.addPostProcessingDataTagsTo(parent)


class ImplicitCouplingScheme(CouplingScheme):

    def __init__(self, timestep, maxTime, maxIterations, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
        self.schemeName = "implicit"
        self.maxIterations = maxIterations

    def addCouplingSchemeTagTo(self, parent):
        couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTagTo(parent)
        self.addMaxIterationsTagTo(couplingSchemeTag)
        postProcessingTag = self.addPostProcessingTagTo(couplingSchemeTag)
        self.addPostProcessingDataTagsTo(postProcessingTag)

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

    def addCouplingParticipantTagsTo(self, parent):
        for participant in self.participants:
            etree.SubElement(parent, "participant", name=participant.name)

    def addExchangeTagsTo(self, parent):
        for pair in self.participantPairs:
            interfaces = pair[0].getInterfacesWith(pair[1])
            for interface in interfaces:
                interface.addExchangeTagsTo(parent)
                interface.partnerInterface.addExchangeTagsTo(parent)
                interface.addPostProcessingDataTagsTo(parent)
