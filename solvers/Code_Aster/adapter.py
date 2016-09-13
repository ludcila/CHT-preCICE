import os
import sys
import numpy as np
import numpy.linalg
from mpi4py import MPI
from Cata.cata import *
from Utilitai.partition import *

np.set_printoptions(threshold=np.inf)

precice_root = os.getenv('PRECICE_ROOT')
precice_python_adapter_root = precice_root+"/src/precice/adapters/python"
sys.path.insert(0, precice_python_adapter_root)

import PySolverInterface
from PySolverInterface import *

class Adapter:
	
	precice = None
	numInterfaces = 0
	interfaces = []
	LOADS = []
	MESH = None
	MODEL = None
	MAT = None
	isNonLinear = False
	
	def __init__(self, precice, config, MESH, MODEL, MAT, isNonLinear=False):
		self.numInterfaces = len(config)
		self.precice = precice
		self.MESH = MESH
		self.MODEL = MODEL
		self.MAT = MAT
		self.configure(config)
		self.isNonLinear = isNonLinear
	
	def configure(self, config):
		L = [None] * self.numInterfaces		# Loads
		SM = [None] * self.numInterfaces	# Shifted meshes
		for i in range(self.numInterfaces):
			# Shifted mesh
			SM[i] = CREA_MAILLAGE(MAILLAGE=self.MESH, RESTREINT={"GROUP_MA": config[i]["groupName"], "GROUP_NO": config[i]["groupName"]})
			# Create interface
			interface = Interface(self.precice, config[i], self.MESH, SM[i], self.MODEL, self.MAT[config[i]["materialID"]], self.isNonLinear)
			# Loads
			BCs = interface.createBCs()
			L[i] = AFFE_CHAR_THER(MODELE=self.MODEL, ECHANGE=BCs)
			interface.setLoad(L[i])
			self.LOADS.append({'CHARGE': L[i]})
			self.interfaces.append(interface)
	
	def sendCouplingData(self, TEMP, dt=0):
		if (dt == 0) or (dt > 0 and self.precice.isWriteDataRequired(dt)):
			for interface in self.interfaces:
				interface.writeBCs(TEMP)
	
	def receiveCouplingData(self):
		if self.precice.isReadDataAvailable():
			for interface in self.interfaces:
				interface.readAndUpdateBCs()

class Interface:
	
	precice = None
	
	
	
	mesh = None
	
	groupName = ""
	groupName = ""
	
	facesMeshName = ""
	nodesMeshName = ""
	
	nodes = []
	faces = []
	connectivity = []
	nodeCoordinates = []
	faceCenterCoordinates = []
	normals = None
	
	isNonLinear = False
	conductivity = None
	conductivityIsInitialized = False
	delta = 1e-5
	
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
	
	# Shifted mesh (contains only the interface, and is shifted by delta in the direction opposite to the normal)
	SHMESH = None
	
	def __init__(self, precice, names, MESH, SHMESH, MODEL, MAT, isNonLinear = False):
		self.precice = precice
		self.MESH = MESH
		self.SHMESH = SHMESH
		self.MODEL = MODEL
		self.MAT = MAT
		self.mesh = MAIL_PY()
		self.mesh.FromAster(MESH)
		self.configure(names)
		self.isNonLinear = isNonLinear
	
	def configure(self, names):
		self.groupName = names["groupName"]
		self.nodesMeshName = names["nodesMeshName"]
		self.faceCentersMeshName = names["faceCentersMeshName"]
		
		self.computeNormals()
		
		self.nodeCoordinates = np.array([p for p in self.SHMESH.sdj.COORDO.VALE.get()])
		self.nodeCoordinates = np.resize(self.nodeCoordinates, (len(self.nodeCoordinates)/3, 3))
		self.shiftMesh()
		
		self.faces = [self.mesh.correspondance_mailles[idx] for idx in self.mesh.gma[self.groupName]]
		self.connectivity = [self.mesh.co[idx] for idx in self.mesh.gma[self.groupName]]
		self.faceCenterCoordinates = np.array([np.array([self.mesh.cn[node] for node in elem]).mean(0) for elem in self.connectivity])

		self.setVertices()
		self.setDataIDs()
		
		self.readDataSize = len(self.faces)
		self.writeDataSize = len(self.nodeCoordinates)
		
		self.readHCoeff = np.zeros(self.readDataSize)
		self.readTemp = np.zeros(self.readDataSize)
	
	def computeNormals(self):
		# Get normals at the nodes
		DUMMY = AFFE_MODELE(
			MAILLAGE=self.SHMESH,
			AFFE={
				'TOUT': 'OUI',
				'PHENOMENE': 'THERMIQUE',
				'MODELISATION': '3D',
			},
		);
		N = CREA_CHAMP(
			MODELE=DUMMY,
			TYPE_CHAM='NOEU_GEOM_R',
			GROUP_MA=self.groupName,
			OPERATION='NORMALE'
		)
		self.normals = N.EXTR_COMP().valeurs
		self.normals = np.resize(np.array(self.normals), (len(self.normals)/3, 3))
		
		"""
		print self.normals
		faces = np.array([np.array([self.mesh.cn[node] for node in elem]) for elem in self.connectivity])
		normals = []
		for face in faces:
			A = face[0]
			B = face[1]
			C = face[2]
			normal = np.cross(B-A, C-A)
			normal = normal / np.linalg.norm(normal)
			normals.append(normal)
		
		print np.array(normals)
		quit()
		"""
		DETRUIRE(CONCEPT=({"NOM": N}, {"NOM": DUMMY}))
	
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
		
	def writeBCs(self, TEMP):
		writeTemp, writeHCoeff = self.getBoundaryValues(TEMP)
		self.precice.writeBlockScalarData(self.writeHCoeffDataID, self.writeDataSize, self.preciceNodeIndices, writeHCoeff)
		self.precice.writeBlockScalarData(self.writeTempDataID, self.writeDataSize, self.preciceNodeIndices, writeTemp)

	def getBoundaryValues(self, T):
		
		# Sink temperature
		TPROJ = PROJ_CHAMP(MAILLAGE_1=self.MESH, MAILLAGE_2=self.SHMESH, CHAM_GD=T, METHODE='COLLOCATION')
		writeTemp = TPROJ.EXTR_COMP(lgno=[self.groupName]).valeurs
		DETRUIRE(CONCEPT=({'NOM': TPROJ}))
		
		# Heat transfer coefficient
		self.updateConductivity(writeTemp)
		writeHCoeff = np.array(self.conductivity) / self.delta
		
		return writeTemp, writeHCoeff
	
	def setLoad(self, LOAD):
		self.LOAD = LOAD
		
	def shiftMesh(self):
		coords = [p for p in self.SHMESH.sdj.COORDO.VALE.get()]
		for i in range(len(self.normals)):
			for c in range(3):
				coords[i*3 + c] = coords[i*3 + c] - self.normals[i][c] * self.delta
		self.SHMESH.sdj.COORDO.VALE.changeJeveuxValues(len(coords), tuple(range(1, len(coords)+1)), tuple(coords), tuple(coords), 1)

	def updateConductivity(self, T):
		if not self.conductivityIsInitialized or self.isNonLinear:
			self.conductivity = [self.MAT.RCVALE("THER", nompar="TEMP", valpar=t, nomres="LAMBDA")[0][0] for t in T]
			self.conductivityIsInitialized = True
			# RCVALE returns ((LAMBDA,),(0,)), therefore we use [0][0] to extract the value of LAMBDA
