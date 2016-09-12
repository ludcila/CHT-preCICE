import os
import sys
import numpy as np
import numpy.linalg
from mpi4py import MPI
from Cata.cata import *
from Utilitai.partition import *

precice_root = os.getenv('PRECICE_ROOT')
precice_python_adapter_root = precice_root+"/src/precice/adapters/python"
sys.path.insert(0, precice_python_adapter_root)

import PySolverInterface
from PySolverInterface import *

class Adapter:
	
	conductivity = 100
	precice = None
	numInterfaces = 0
	interfaces = []
	LOADS = []
	MESH = None
	MODEL = None
	
	def __init__(self, precice, config, MESH, MODEL):
		self.numInterfaces = len(config)
		self.precice = precice
		self.MESH = MESH
		self.MODEL = MODEL
		self.configure(config)
	
	def configure(self, config):
		L = [None] * self.numInterfaces
		for i in range(self.numInterfaces):
			interface = Interface(self.precice, config[i], self.MESH, self.MODEL)
			BCs = interface.createBCs()
			L[i] = AFFE_CHAR_THER(MODELE=self.MODEL, ECHANGE=BCs)
			interface.setLoad(L[i])
			interface.setConductivity(np.ones(interface.writeDataSize) * self.conductivity)
			self.LOADS.append({"CHARGE": L[i]})
			self.interfaces.append(interface)
	
	def sendCouplingData(self, TEMP, dt=0, NUME_ORDRE=0):
		if (dt == 0) or (dt > 0 and self.precice.isWriteDataRequired(dt)):
			for interface in self.interfaces:
				interface.writeBCs(TEMP, NUME_ORDRE)
	
	def receiveCouplingData(self):
		if self.precice.isReadDataAvailable():
			for interface in self.interfaces:
				interface.readAndUpdateBCs()
				
	def setConductivity(self, cond):
		self.conductivity = cond

class Interface:
	
	precice = None
	
	mesh = None
	
	groupName = ""
	groupName = ""
	
	facesMeshName = ""
	nodesMeshName = ""
	
	nodes = []
	faces = []
	nodeCoordinates = []
	faceCenterCoordinates = []
	normals = None
	
	conductivity = None
	delta = 1e-6
	
	preciceNodeIndices = []
	preciceFaceCenterIndices = []
	
	preciceFaceCentersMeshID = 0
	preciceNodesMeshID = 0
	
	readHCoeffDataID = 0
	readTempDataID = 0
	writeHCoeffDataID = 0
	writeTempDataID = 0
	
	readTemp = []
	readHCoeff = []
	writeTemp = []
	writeHCoeff = []
	
	readDataSize = 0
	writeDataSize = 0
	
	MESH = None
	MODEL = None
	LOAD = None
	LOADS = []
	
	def __init__(self, precice, names, MESH, MODEL):
		self.precice = precice
		self.MESH = MESH
		self.MODEL = MODEL
		self.mesh = MAIL_PY()
		self.mesh.FromAster(MESH)
		self.configure(names)
	
	def configure(self, names):
		self.groupName = names["groupName"]
		self.nodesMeshName = names["nodesMeshName"]
		self.faceCentersMeshName = names["faceCentersMeshName"]
		self.nodes = [self.mesh.correspondance_noeuds[idx] for idx in self.mesh.gno[self.groupName]]
		self.faces = [self.mesh.correspondance_mailles[idx] for idx in self.mesh.gma[self.groupName]]
		self.nodeCoordinates = [self.mesh.cn[idx] for idx in self.mesh.gno[self.groupName]]
		connectivity = [self.mesh.co[idx] for idx in self.mesh.gma[self.groupName]]
		self.faceCenterCoordinates = np.array([np.array([self.mesh.cn[node] for node in elem]).mean(0) for elem in connectivity])
		self.setNormals()
		self.setVertices()
		self.setDataIDs()
		
		self.readDataSize = len(self.faces)
		self.writeDataSize = len(self.nodes)
		
		self.readHCoeff = np.zeros(self.readDataSize)
		self.readTemp = np.zeros(self.readDataSize)
	
	def setNormals(self):
		# Get normals at the nodes
		N = CREA_CHAMP(
			MODELE=self.MODEL,
			TYPE_CHAM='NOEU_GEOM_R',
			GROUP_MA=self.groupName,
			OPERATION='NORMALE'
		)
		self.normals = N.EXTR_COMP(lgno=[self.groupName]).valeurs
		self.normals = np.resize(np.array(self.normals), (len(self.normals)/3, 3))
		DETRUIRE(CONCEPT={"NOM": N})
	
	def setVertices(self):
		# Nodes
		self.preciceNodeIndices = [0] * len(self.nodeCoordinates)
		self.preciceNodesMeshID = self.precice.getMeshID(self.nodesMeshName)
		self.precice.setMeshVertices(self.preciceNodesMeshID, len(self.nodeCoordinates), np.hstack(self.nodeCoordinates), self.preciceNodeIndices)
		# Face centers
		self.preciceFaceCenterIndices = [0] * len(self.faceCenterCoordinates)
		self.preciceFaceCentersMeshID = self.precice.getMeshID(self.faceCentersMeshName)
		self.precice.setMeshVertices(self.preciceFaceCentersMeshID, len(self.faceCenterCoordinates), np.hstack(self.faceCenterCoordinates), self.preciceFaceCenterIndices)
		
	def setDataIDs(self):
		self.readHCoeffDataID = self.precice.getDataID("kDelta-OF", self.preciceFaceCentersMeshID)
		self.readTempDataID = self.precice.getDataID("kDelta-Temperature-OF", self.preciceFaceCentersMeshID)
		self.writeHCoeffDataID = self.precice.getDataID("kDelta-CCX", self.preciceNodesMeshID)
		self.writeTempDataID = self.precice.getDataID("kDelta-Temperature-CCX", self.preciceNodesMeshID)
	
	def getPreciceNodeIndices(self):
		return self.preciceNodeIndices
	
	def getPreciceFaceCenterIndices(self):
		return self.preciceFaceCenterIndices
	
	def getPreciceNodesMeshID(self):
		return self.preciceNodesMeshID
	
	def getPreciceFaceCentersMeshID(self):
		return self.preciceFaceCentersMeshID
	
	def getNodes(self):
		return self.nodes
	
	def getFaces(self):
		return self.faces
	
	def getNodeCoordinates(self):
		return self.nodeCoordinates
	
	def getFaceCenterCoordinates(self):
		return self.faceCenterCoordinates
		
	def getNormals(self):
		return self.normals
	
	def createBCs(self):
		"""
		Note: TEMP_EXT and COEF_H need to be initialized with different values, otherwise Code_Aster
		will group identical values together, and it will not be possible to apply different BCs
		to different element faces.  Additionally, COEF_H must be different from 0
		(otherwise it will be grouped with a default internal 0).
		"""
		BCs = [
			{'MAILLE': self.faces[j], 'TEMP_EXT': j, 'COEF_H': j+1}
			for j in range(len(self.faces))
		]
		return BCs
	
	
	def updateBCs(self, temp, hCoeff):
			
		self.LOAD.sdj.CHTH.T_EXT.VALE.changeJeveuxValues(len(temp),
										tuple(np.array(range(len(temp))) * 10 + 1),
										tuple(temp),
										tuple(temp),
										1)
		self.LOAD.sdj.CHTH.COEFH.VALE.changeJeveuxValues(len(hCoeff),
										tuple(np.array(range(1, len(hCoeff)+1)) * 3 + 1),
										tuple(hCoeff),
										tuple(hCoeff),
										1)
										
	def readAndUpdateBCs(self):
		self.precice.readBlockScalarData(self.readHCoeffDataID, self.readDataSize, self.preciceFaceCenterIndices, self.readHCoeff)
		self.precice.readBlockScalarData(self.readTempDataID, self.readDataSize, self.preciceFaceCenterIndices, self.readTemp)
		self.updateBCs(self.readTemp, self.readHCoeff)
		
	def writeBCs(self, TEMP, NUME_ORDRE=0):
		writeTemp, writeHCoeff = self.getBoundaryValues(TEMP, NUME_ORDRE)
		self.precice.writeBlockScalarData(self.writeHCoeffDataID, self.writeDataSize, self.preciceNodeIndices, writeHCoeff)
		self.precice.writeBlockScalarData(self.writeTempDataID, self.writeDataSize, self.preciceNodeIndices, writeTemp)

	def getBoundaryValues(self, TEMP, NUME_ORDRE=0):
		if self.conductivity is None:
			print "ERROR: Call setConductivity(conductivity) before calling getBoundaryValues()!"
			exit(1)
			
		# Temperature at the nodes
		T = CREA_CHAMP(
			RESULTAT=TEMP,
			NOM_CHAM='TEMP',
			TYPE_CHAM='NOEU_TEMP_R',
			OPERATION='EXTR',
			NUME_ORDRE=NUME_ORDRE,
		)
		
		# Flux at the nodes
		Q = CREA_CHAMP(
			RESULTAT=TEMP,
			NOM_CHAM='FLUX_NOEU',
			TYPE_CHAM='NOEU_FLUX_R',
			OPERATION='EXTR',
			NUME_ORDRE=NUME_ORDRE,
		)
		
		t = T.EXTR_COMP(lgno=[self.groupName]).valeurs
		q = Q.EXTR_COMP(lgno=[self.groupName]).valeurs
		q = np.resize(np.array(q), (len(q)/3, 3))
		
		DETRUIRE(CONCEPT=({'NOM': T}, {'NOM': Q}))
		
		# Surface normal flux (flux dot normal)
		snq = (self.normals * q).sum(1)
		# Sink temperature
		writeTemp = t + snq * self.delta
		# Heat transfer coefficient
		writeHCoeff = np.array(self.conductivity) / self.delta
		return writeTemp, writeHCoeff
	
	def setLoad(self, LOAD):
		self.LOAD = LOAD
		
	def setConductivity(self, conductivity):
		self.conductivity = conductivity
