import yaml
from xml.dom import minidom

pretty_print = lambda data: '\n'.join([line for line in data.toprettyxml(indent='\t').split('\n') if line.strip()])

class Config:

	participants = []
	couplings = []
	interfaceToParticipantMap = None
	interfaceToInterfaceMap = None
	interfaceToSideMap = None
	participantToSolverMap = None

	def __init__(self, config):
		self.config = config
		self.createInterfaceToParticipantMap()
		self.createInterfaceToInterfaceMap()
		self.createParticipantToSolverMap()
		self.createCouplings()

	def addParticipant(self, participant):
		self.participants.append(participant)

	# Creates a mapping from the interface name to the participant who owns this interface
	def createInterfaceToParticipantMap(self):
		self.interfaceToParticipantMap = {}
		for participant in self.config.get("participants"):
			for interface in self.config.get("participants")[participant]["interfaces"]:
				self.interfaceToParticipantMap[interface["name"]] = participant

	# Creates a mapping from one interface to the interface it is coupled with
	def createInterfaceToInterfaceMap(self):
		self.interfaceToInterfaceMap = {}
		self.interfaceToSideMap = {}
		for coupling in self.config.get("couplings"):
			self.interfaceToInterfaceMap[coupling["A"]] = coupling["B"]
			self.interfaceToInterfaceMap[coupling["B"]] = coupling["A"]
			self.interfaceToSideMap[coupling["A"]] = "A"
			self.interfaceToSideMap[coupling["B"]] = "B"
			# TODO: check 1-to-1 coupling

	def createParticipantToSolverMap(self):
		self.participantToSolverMap = {}
		for participant in self.config.get("participants"):
			self.participantToSolverMap[participant] = self.config.get("participants")[participant]["solver"]

	def createCouplings(self):
		for coupling in self.config.get("couplings"):
			A = self.getParticipantFromInterfaceName(coupling["A"])
			B = self.getParticipantFromInterfaceName(coupling["B"])
			pair = [A, B]
			pair.sort()
			self.couplings.append(pair)
		self.couplings = [list(p) for p in set(tuple(p) for p in self.couplings)]

	def getInterfacesByParticipantName(self, participantName):
		return self.config.get("participants")[participantName]["interfaces"]

	def getParticipantFromInterfaceName(self, participantName):
		return self.interfaceToParticipantMap[participantName]

	def getPartnerInterfaceName(self, interfaceName):
		return self.interfaceToInterfaceMap[interfaceName]

	def getSolverFromParticipant(self, participantName):
		return self.participantToSolverMap[participantName]

	def getSideFromInterfaceName(self, interfaceName):
		return self.interfaceToSideMap[interfaceName]

	def getParticipantByName(self, participantName):
		for participant in self.participants:
			if participant.name == participantName:
				return participant

class Participant:

	def __init__(self, config, name):
		self.name = name
		self.config = config
		self.interfaces = self.config.getInterfacesByParticipantName(self.name)
		self.couplingsByInterface = {}
		self.couplingsByPartner = {}
		self.config.addParticipant(self)
		self.determineCouplings()

	# Not necessary??
	def determineCouplings(self):
		for interface in self.interfaces:
			myInterface = interface["name"]
			partnerInterface = self.config.getPartnerInterfaceName(myInterface)
			partnerParticipant = self.config.getParticipantFromInterfaceName(partnerInterface)
			self.couplingsByInterface[myInterface] = {"partnerInterface": partnerInterface, "partnerParticipant": partnerParticipant}
			temp = {"interface": myInterface, "partnerInterface": partnerInterface}
			if not self.couplingsByPartner.has_key(partnerParticipant):
				self.couplingsByPartner[partnerParticipant] = [temp]
			else:
				self.couplingsByPartner[partnerParticipant].append(temp)

	def addMeshTag(self, xmldoc):
		solverInterface = xmldoc.getElementsByTagName("solver-interface")[0]
		# mesh
		for interface in self.interfaces:
			interface = interface["name"]
			mesh = xmldoc.createElement("mesh")
			mesh.setAttribute("name", interface)
			data = xmldoc.createElement("use-data")
			data.setAttribute("name", "Heat-Transfer-Coefficient-A")
			mesh.appendChild(data)
			data = xmldoc.createElement("use-data")
			data.setAttribute("name", "Heat-Transfer-Coefficient-B")
			mesh.appendChild(data)
			data = xmldoc.createElement("use-data")
			data.setAttribute("name", "Sink-Temperature-A")
			mesh.appendChild(data)
			data = xmldoc.createElement("use-data")
			data.setAttribute("name", "Sink-Temperature-B")
			mesh.appendChild(data)
			solverInterface.appendChild(mesh)

	def addParticipantTag(self, xmldoc):
		self.solverInterface = xmldoc.getElementsByTagName("solver-interface")[0]

		# <participant>
		participant = xmldoc.createElement("participant")
		participant.setAttribute("name", self.name)

		for interface in self.interfaces:

			interface = interface["name"]
			partnerInterface = self.config.getPartnerInterfaceName(interface)
			partnerParticipant = self.config.getParticipantFromInterfaceName(partnerInterface)
			side = self.config.getSideFromInterfaceName(interface)
			oppositeSide = "B" if side == "A" else "A"

			# <use-mesh>
			myMesh = xmldoc.createElement("use-mesh")
			myMesh.setAttribute("name", interface)
			myMesh.setAttribute("provide", "yes")
			participant.appendChild(myMesh)

			# <use-mesh>
			othersMesh = xmldoc.createElement("use-mesh")
			othersMesh.setAttribute("name", partnerInterface)
			othersMesh.setAttribute("from", partnerParticipant)
			participant.appendChild(othersMesh)

			# <write-data>
			writeData = xmldoc.createElement("write-data")
			writeData.setAttribute("name", "Heat-Transfer-Coefficient-" + side)
			writeData.setAttribute("mesh", interface)
			participant.appendChild(writeData)
			writeData = xmldoc.createElement("write-data")
			writeData.setAttribute("name", "Sink-Temperature-" + side)
			writeData.setAttribute("mesh", interface)
			participant.appendChild(writeData)

			# <read-data>
			readData = xmldoc.createElement("read-data")
			readData.setAttribute("name", "Heat-Transfer-Coefficient-" + oppositeSide)
			readData.setAttribute("mesh", interface)
			participant.appendChild(readData)
			readData = xmldoc.createElement("read-data")
			readData.setAttribute("name", "Sink-Temperature-" + oppositeSide)
			readData.setAttribute("mesh", interface)
			participant.appendChild(readData)

			# <mapping>
			mapping = xmldoc.createElement("mapping:nearest-neighbour")
			mapping.setAttribute("from", partnerInterface)
			mapping.setAttribute("to", interface)
			mapping.setAttribute("direction", "read")
			participant.appendChild(mapping)

		self.solverInterface.appendChild(participant)


xmldoc = minidom.parse("template.xml")

configFile = yaml.load(open("config.yml").read())
config = Config(configFile)

participantNames = ["Water", "Hot-Plate", "Cold-Plate"]
participants = []

for participantName in participantNames:
	p = Participant(config, participantName)
	participants.append(p)

for participant in participants:
	participant.addMeshTag(xmldoc)

for participant in participants:
	participant.addParticipantTag(xmldoc)


for pNames in config.couplings:

	p1 = config.getParticipantByName(pNames[0])
	p2 = config.getParticipantByName(pNames[1])

	coupling = xmldoc.createElement("coupling-scheme")

	# time-step-length
	timestep = xmldoc.createElement("timestep-length")
	timestep.setAttribute("value", "1")
	coupling.appendChild(timestep)

	# max-time
	timestep = xmldoc.createElement("max-time")
	timestep.setAttribute("value", "1")
	coupling.appendChild(timestep)

	# participants
	participants = xmldoc.createElement("participants")
	participants.setAttribute("first", pNames[0])
	participants.setAttribute("second", pNames[1])
	coupling.appendChild(participants)

	solverInterface = xmldoc.getElementsByTagName("solver-interface")[0]
	solverInterface.appendChild(coupling)

	coupledInterfaces = p1.couplingsByPartner[p2.name]
	for coupledInterface in coupledInterfaces:
		# From self to partner
		exchange = xmldoc.createElement("exchange")
		exchange.setAttribute("data", "Heat-Transfer-Coeff-")
		exchange.setAttribute("from", coupledInterface["interface"])
		exchange.setAttribute("to", coupledInterface["partnerInterface"])
		coupling.appendChild(exchange)
		exchange = xmldoc.createElement("exchange")
		exchange.setAttribute("data", "Sink-Temperature-")
		exchange.setAttribute("from", coupledInterface["interface"])
		exchange.setAttribute("to", coupledInterface["partnerInterface"])
		coupling.appendChild(exchange)
		# From parter to me
		exchange = xmldoc.createElement("exchange")
		exchange.setAttribute("data", "Heat-Transfer-Coeff-")
		exchange.setAttribute("from", coupledInterface["partnerInterface"])
		exchange.setAttribute("to", coupledInterface["interface"])
		coupling.appendChild(exchange)
		exchange = xmldoc.createElement("exchange")
		exchange.setAttribute("data", "Sink-Temperature-")
		exchange.setAttribute("from", coupledInterface["partnerInterface"])
		exchange.setAttribute("to", coupledInterface["interface"])
		coupling.appendChild(exchange)

		# For implicit coupling
		# Max iterations
		maxIterations = xmldoc.createElement("max-iterations")
		maxIterations.setAttribute("value", "50")
		coupling.appendChild(maxIterations)
		# Convergence measure
		convergenceMeasure = xmldoc.createElement("relative-convergence-measure")
		convergenceMeasure.setAttribute("data", "Sink-Temperature-A")
		convergenceMeasure.setAttribute("mesh", "adsf")
		convergenceMeasure.setAttribute("data", "Sink-Temperature-B")
		convergenceMeasure.setAttribute("mesh", "asdf")
		coupling.appendChild(convergenceMeasure)


outputFile = open("test.xml", "w")
outputFile.write(pretty_print(xmldoc))
outputFile.close()
