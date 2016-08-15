#include <stdlib.h>
#include "PreciceInterface.h"
#include "ConfigReader.h"
#include "precice/adapters/c/SolverInterfaceC.h"


void PreciceInterface_Setup(char * configFilename, char * participantName, struct CalculiXData ccx, struct PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces)
{
	char * preciceConfigFilename;

	CoupledSurfaceConfig * coupledSurfaces;

	ConfigReader_Read(configFilename, participantName, &preciceConfigFilename, &coupledSurfaces, numPreciceInterfaces);
	precicec_createSolverInterface(participantName, preciceConfigFilename, 0, 1);

	*preciceInterfaces = (struct PreciceInterface **) malloc(*numPreciceInterfaces * sizeof(struct PreciceInterface *)); // TODO: free memory

	int i;
	for(i = 0; i < *numPreciceInterfaces; i++) {
		(*preciceInterfaces)[i] = malloc(sizeof(struct PreciceInterface));
		PreciceInterface_CreateInterface((*preciceInterfaces)[i], ccx, coupledSurfaces[i]);
	}
}

void PreciceInterface_CreateInterface(struct PreciceInterface * interface, struct CalculiXData ccx, const CoupledSurfaceConfig config) {

	interface->name = config.patchNames[0];

	// Nodes mesh
	interface->nodesMeshName = config.nodesMeshName;
	interface->hasNodesMesh = config.hasNodesMesh;
	PreciceInterface_ConfigureNodesMesh(interface, ccx);

	// Face centers mesh
	interface->faceCentersMeshName = config.facesMeshName;
	interface->hasFaceCentersMesh = config.hasFacesMesh;
	PreciceInterface_ConfigureFaceCentersMesh(interface, ccx);

	// Triangles of the nodes mesh (needs to be called after the face centers mesh is configured!)
	PreciceInterface_ConfigureTetraFaces(interface, ccx);

	// Configure read data (boundary conditions to be read from preCICE)
	int i = 0;
	for(i = 0; i < config.numReadData; i++) {
		if(strcmp(config.readData[i], "Temperature") == 0) {
			interface->xbounIndices = malloc(interface->numNodes * sizeof(int));
			getXbounIndices(interface->nodeIDs, interface->numNodes, ccx.nboun, ccx.ikboun, ccx.ilboun, interface->xbounIndices);	
		} else if(strcmp(config.readData[i], "Heat-Flux") == 0) {
			interface->xloadIndices = malloc(interface->numElements * sizeof(int));
			getXloadIndices("DFLUX", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices);
		} else if(strcmp(config.readData[i], "Sink-Temperature") == 0 || strcmp(config.readData[i], "Heat-Transfer-Coefficient") == 0) {
			interface->xloadIndices = malloc(interface->numElements * sizeof(int));
			getXloadIndices("FILM", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices);
		}
	}
	// Configure write data (boundary values to be sent to preCICE)
	for(i = 0; i < config.numWriteData; i++) {
		
	}
	
	if(strcmp(config.readDataName, "Temperature") == 0) {
		interface->readData = TEMPERATURE;
	} else if(strcmp(config.readDataName, "Heat-Flux") == 0) {
		interface->readData = HEAT_FLUX;
	} else if(strcmp(config.readDataName, "Sink-Temperature") == 0) {
		interface->readData = SINK_TEMPERATURE;
	} else if(strcmp(config.readDataName, "kDelta-Temperature") == 0) {
		interface->readData = KDELTA_TEMPERATURE;
	}
	
	if(strcmp(config.writeDataName, "Temperature") == 0) {
		interface->writeData = TEMPERATURE;
	} else if(strcmp(config.writeDataName, "Heat-Flux") == 0) {
		interface->writeData = HEAT_FLUX;
	} else if(strcmp(config.writeDataName, "kDelta-Temperature") == 0) {
		interface->writeData = KDELTA_TEMPERATURE;
	}
	PreciceInterface_ConfigureHeatTransferData(interface, ccx);

}

void PreciceInterface_ConfigureFaceCentersMesh(struct PreciceInterface * interface, struct CalculiXData ccx) {

	char * faceSetName = toFaceSetName(interface->name);
	interface->faceSetID = getSetID(faceSetName, ccx.set, ccx.nset);
	interface->numElements = getNumSetElements(interface->faceSetID, ccx.istartset, ccx.iendset);

	interface->elementIDs = malloc(interface->numElements * sizeof(ITG));
	interface->faceIDs = malloc(interface->numElements * sizeof(ITG));
	getSurfaceElementsAndFaces(interface->faceSetID, ccx.ialset, ccx.istartset, ccx.iendset, interface->elementIDs, interface->faceIDs);

	interface->faceCenterCoordinates = malloc(interface->numElements * 3 * sizeof(double));
	getTetraFaceCenters(interface->elementIDs, interface->faceIDs, interface->numElements, ccx.kon, ccx.ipkon, ccx.co, interface->faceCenterCoordinates);

	if(interface->hasFaceCentersMesh){
		interface->faceCentersMeshID = precicec_getMeshID(interface->faceCentersMeshName);
		interface->preciceFaceCenterIDs = malloc(interface->numElements * sizeof(int));
		precicec_setMeshVertices(interface->faceCentersMeshID, interface->numElements, interface->faceCenterCoordinates, interface->preciceFaceCenterIDs);
	}
	
}

void PreciceInterface_ConfigureNodesMesh(struct PreciceInterface * interface, struct CalculiXData ccx) {

	char * nodeSetName = toNodeSetName(interface->name);
	interface->nodeSetID = getSetID(nodeSetName, ccx.set, ccx.nset);
	interface->numNodes = getNumSetElements(interface->nodeSetID, ccx.istartset, ccx.iendset);
	interface->nodeIDs = &ccx.ialset[ccx.istartset[interface->nodeSetID] - 1]; // TODO: make a copy

	interface->nodeCoordinates = malloc(interface->numNodes * 3 * sizeof(double));
	getNodeCoordinates(interface->nodeIDs, interface->numNodes, ccx.co, interface->nodeCoordinates);

	if(interface->hasNodesMesh) {
		interface->nodesMeshID = precicec_getMeshID(interface->nodesMeshName);
		interface->preciceNodeIDs = malloc(interface->numNodes * sizeof(int));
		precicec_setMeshVertices(interface->nodesMeshID, interface->numNodes, interface->nodeCoordinates, interface->preciceNodeIDs);
	}
	
}

void PreciceInterface_ConfigureTetraFaces(struct PreciceInterface * interface, struct CalculiXData ccx) {
	if(interface->nodesMeshName) {
		interface->triangles = malloc(interface->numElements * 3 * sizeof(ITG));
		getTetraFaceNodes(interface->elementIDs, interface->faceIDs,  interface->nodeIDs, interface->numElements, interface->numNodes, ccx.kon, ccx.ipkon, interface->triangles);
		int i;
		for(i = 0; i < interface->numElements; i++) {
			precicec_setMeshTriangleWithEdges(interface->nodesMeshID, interface->triangles[3*i], interface->triangles[3*i+1], interface->triangles[3*i+2]);
		}
	}
}


void PreciceInterface_ConfigureHeatTransferData(struct PreciceInterface * interface, struct CalculiXData ccx) {
	
	interface->nodeData = malloc(interface->numNodes * sizeof(double));
	interface->faceCenterData = malloc(interface->numElements * sizeof(double));
	
	// Get the indices where the boundary conditions must be set
	if(interface->readData == TEMPERATURE) {
	} else if(interface->readData == HEAT_FLUX) {
	} else if(interface->readData == SINK_TEMPERATURE) {
		interface->xloadIndices = malloc(interface->numElements * sizeof(int));
		getXloadIndices("FILM", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices);
	} else if(interface->readData == KDELTA_TEMPERATURE) {
		interface->xloadIndices = malloc(interface->numElements * sizeof(int));
		getXloadIndices("FILM", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices);
	}
	
	// Get data ID's from preCICE
	if(interface->hasNodesMesh && precicec_hasData("Temperature", interface->nodesMeshID)) {
		interface->temperatureDataID = precicec_getDataID("Temperature", interface->nodesMeshID);
	}
	if(interface->hasFaceCentersMesh && precicec_hasData("Heat-Flux", interface->faceCentersMeshID)) {
		interface->fluxDataID = precicec_getDataID("Heat-Flux", interface->faceCentersMeshID);
	}
	if(interface->hasFaceCentersMesh && precicec_hasData("Sink-Temperature", interface->faceCentersMeshID)) {
		interface->sinkTemperatureDataID = precicec_getDataID("Sink-Temperature", interface->faceCentersMeshID);
	}
	// TODO: find a smarter way to deal with this 
	// (the adapter should not need to know who are the other participants)
	if(interface->hasFaceCentersMesh && precicec_hasData("kDelta-CCX", interface->faceCentersMeshID)) {
		interface->kDeltaWriteDataID = precicec_getDataID("kDelta-CCX", interface->faceCentersMeshID);
		interface->kDeltaReadDataID = precicec_getDataID("kDelta-OF", interface->faceCentersMeshID);
		interface->kDeltaTemperatureWriteDataID = precicec_getDataID("kDelta-Temperature-CCX", interface->faceCentersMeshID);
		interface->kDeltaTemperatureReadDataID = precicec_getDataID("kDelta-Temperature-OF", interface->faceCentersMeshID);
	}
	
}

void PreciceInterface_WriteIterationCheckpoint(CalculiXData * ccx, double * v) {

	// Save time
	ccx->coupling_init_theta = *(ccx->theta);

	// Save step size
	ccx->coupling_init_dtheta = *(ccx->dtheta);

	// Save solution vector v
	memcpy(ccx->coupling_init_v, v, sizeof(double) * ccx->mt * ccx->nk);

}

void PreciceInterface_ReadIterationCheckpoint(CalculiXData * ccx, double * v) {

	// Reload time
	*(ccx->theta) = ccx->coupling_init_theta;

	// Reload step size
	*(ccx->dtheta) = ccx->coupling_init_dtheta;

	// Reload solution vector v
	memcpy(v, ccx->coupling_init_v, sizeof(double) * ccx->mt * ccx->nk);

}

void PreciceInterface_AdjustSolverTimestep(double precice_dt, double tper, double *dtheta, double *solver_dt) {
	printf("precice_dt dtheta = %f, dtheta = %f, solver_dt = %f\n", precice_dt / tper, *dtheta, fmin(precice_dt, *dtheta * tper));
	*dtheta = fmin(precice_dt / tper, *dtheta);
	*solver_dt = *dtheta * tper;
}

void PreciceInterface_DeallocateAll() {
	// Deallocate the data structures of each interface
}
