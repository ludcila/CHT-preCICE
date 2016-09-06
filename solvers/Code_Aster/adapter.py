import os
import sys
import numpy as np
import numpy.linalg
from mpi4py import MPI
from Utilitai.partition import *

precice_root = os.getenv('PRECICE_ROOT')
precice_python_adapter_root = precice_root+"/src/precice/adapters/python"
sys.path.insert(0, precice_python_adapter_root)

import PySolverInterface
from PySolverInterface import *

class Interface:
	
	precice = None
	
	mesh = MAIL_PY()
	
	facesGroupName = ""
	nodesGroupName = ""
	
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
	
	LOAD = None
	
	def __init__(self, precice):
		self.precice = precice
	
	def configure(self, data):
		self.nodesGroupName = data["nodesGroupName"]
		self.facesGroupName = data["facesGroupName"]
		self.nodesMeshName = data["nodesMeshName"]
		self.faceCentersMeshName = data["faceCentersMeshName"]
		self.nodes = [self.mesh.correspondance_noeuds[idx] for idx in self.mesh.gno[self.nodesGroupName]]
		self.faces = [self.mesh.correspondance_mailles[idx] for idx in self.mesh.gma[self.facesGroupName]]
		self.nodeCoordinates = [self.mesh.cn[idx] for idx in self.mesh.gno[self.nodesGroupName]]
		connectivity = [self.mesh.co[idx] for idx in self.mesh.gma[self.facesGroupName]]
		self.faceCenterCoordinates = [(
			self.mesh.cn[connectivity[i][0]] + 
			self.mesh.cn[connectivity[i][1]] + 
			self.mesh.cn[connectivity[i][2]] ) / 3 
			for i in range(len(connectivity))]
		self.setVertices()
		self.setDataIDs()
		
		self.readDataSize = len(self.faces)
		self.writeDataSize = len(self.nodes)
		
		self.readHCoeff = np.zeros(self.readDataSize)
		self.readTemp = np.zeros(self.readDataSize)
	
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
	
	def setNormals(self, normals):
		self.normals = normals
		
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
		
		if self.LOAD is None:
			print "ERROR: LOAD not defined. Call setAsterLoad(LOAD) first, before trying to update the BCs!"
			exit(1)
			
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
		
	def writeBCs(self, T, Q):
		writeTemp, writeHCoeff = self.getBoundaryValues(T, Q)
		self.precice.writeBlockScalarData(self.writeHCoeffDataID, self.writeDataSize, self.preciceNodeIndices, writeHCoeff)
		self.precice.writeBlockScalarData(self.writeTempDataID, self.writeDataSize, self.preciceNodeIndices, writeTemp)

	def getBoundaryValues(self, T, Q):
		if self.normals is None:
			print "ERROR: Call setNormals(normals) before calling getBoundaryValues()!"
			exit(1)
		if self.conductivity is None:
			print "ERROR: Call setConductivity(conductivity) before calling getBoundaryValues()!"
			exit(1)
		t = T.EXTR_COMP(lgno=[self.nodesGroupName]).valeurs
		q = Q.EXTR_COMP(lgno=[self.nodesGroupName]).valeurs
		q = np.resize(np.array(q), (len(q)/3, 3))
		# Surface normal flux
		snq = (self.normals * q).sum(1)
		# Sink temperature
		writeTemp = t + snq * self.delta
		# Heat transfer coefficient
		writeHCoeff = np.array(self.conductivity) / self.delta
		return writeTemp, writeHCoeff
		
	def setConductivity(self, conductivity):
		self.conductivity = conductivity

	def setAsterLoad(self, LOAD):
		self.LOAD = LOAD
