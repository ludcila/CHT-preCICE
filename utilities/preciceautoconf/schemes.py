from lxml import etree
from xmlns import *

class CouplingScheme(object):
    def __init__(self, timestep, maxTime, participants, serial=False):
        self.timestep = timestep
        self.maxTime = maxTime
        self.participants = participants
        self.serial = serial
        if not serial:
            scheme = "serial"
        else:
            scheme = "parallel"
        self.type = "explicit"
        self.schemeName = scheme + "-" + self.type

    def addCouplingSchemeTag(self, parent):
        couplingSchemeTag = etree.SubElement(parent, etree.QName(XMLNamespaces.couplingScheme, self.schemeName))
        self.addTimestepTag(couplingSchemeTag)
        self.addMaxTimeTag(couplingSchemeTag)
        self.addCouplingParticipantTags(couplingSchemeTag)
        self.addExchangeTags(couplingSchemeTag)
        return couplingSchemeTag

    def addTimestepTag(self, parent):
        etree.SubElement(parent, "timestep-length", value=str(self.timestep))

    def addMaxTimeTag(self, parent):
        etree.SubElement(parent, "max-time", value=str(self.maxTime))

    def addCouplingParticipantTags(self, parent):
        etree.SubElement(parent, "participants", first=self.participants[0].name, second=self.participants[1].name)

    def addExchangeTags(self, parent):
        interfaces = self.participants[0].getInterfacesWith(self.participants[1])
        for interface in interfaces:
            interface.addExchangeTags(parent)
            interface.partnerInterface.addExchangeTags(parent)
            interface.addPostProcessingDataTags(parent)


class ImplicitCouplingScheme(CouplingScheme):

    def __init__(self, timestep, maxTime, maxIterations, participants, serial=False):
        super(ImplicitCouplingScheme, self).__init__(timestep, maxTime, participants, serial)
        self.schemeName = "implicit"
        self.maxIterations = maxIterations

    def addCouplingSchemeTag(self, parent):
        couplingSchemeTag = super(ImplicitCouplingScheme, self).addCouplingSchemeTag(parent)
        self.addMaxIterationsTag(couplingSchemeTag)
        postProcessingTag = self.addPostProcessingTag(couplingSchemeTag)
        self.addPostProcessingDataTags(postProcessingTag)

    def getRelativeConvergenceMeasureTags(self):
        pass

    def addMaxIterationsTag(self, parent):
        print self.maxIterations
        etree.SubElement(parent, "max-iterations", value=str(self.maxIterations))

    def addPostProcessingTag(self, parent):
        return etree.SubElement(parent, etree.QName(XMLNamespaces.postProcessing, "IQN-ILS"))

    def addPostProcessingDataTags(self, parent):
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

    def addCouplingParticipantTags(self, parent):
        for participant in self.participants:
            etree.SubElement(parent, "participant", name=participant.name)

    def addExchangeTags(self, parent):
        for pair in self.participantPairs:
            interfaces = pair[0].getInterfacesWith(pair[1])
            for interface in interfaces:
                interface.addExchangeTags(parent)
                interface.partnerInterface.addExchangeTags(parent)
                interface.addPostProcessingDataTags(parent)