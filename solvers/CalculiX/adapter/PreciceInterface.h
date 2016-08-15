#ifndef PRECICEINTERFACE_H
#define PRECICEINTERFACE_H

#include "ConfigReader.h"
#include "CCXHelpers.h"
#include "Helpers.h"

static int TEMPERATURE = 1;
static int HEAT_FLUX = 2;
static int SINK_TEMPERATURE = 3;
static int KDELTA_TEMPERATURE = 4;

typedef struct CalculiXData {
	ITG * ialset;
	ITG * istartset;
	ITG * iendset;
	ITG * kon;
	ITG * ipkon;
	ITG nset;
	char * set;
	double * co;
	ITG nboun;
	ITG * ikboun;
	ITG * ilboun;
	ITG * nelemload;
	int nload;
	char * sideload;
	double nk;
	double mt;
	double * theta;
	double * dtheta;
	double * coupling_init_v;
	double coupling_init_theta;
	double coupling_init_dtheta;
} CalculiXData;

typedef struct PreciceInterface {

	char * name;

	int numNodes;
	int * nodeIDs;
	double * nodeCoordinates;
	int nodeSetID;
	int * preciceNodeIDs;
	int nodesMeshID;
	char * nodesMeshName;
	int hasNodesMesh;

	int numElements;
	int * elementIDs;
	int * faceIDs;
	double * faceCenterCoordinates;
	int faceSetID;
	int faceCentersMeshID;
	char * faceCentersMeshName;
	int * preciceFaceCenterIDs;
	int * triangles;
	int hasFaceCentersMesh;

	double * nodeData;
	double * faceCenterData;

	// preCICE Data IDs
	int temperatureDataID;
	int fluxDataID;
	int sinkTemperatureDataID;
	int kDeltaWriteDataID;
	int kDeltaTemperatureWriteDataID;
	int kDeltaReadDataID;
	int kDeltaTemperatureReadDataID;

	int * xloadIndices;
	int * xbounIndices;
	
	int readData;
	int writeData;

} PreciceInterface;

void PreciceInterface_Setup(char * configFilename, char * participantName, struct CalculiXData ccx, PreciceInterface *** preciceInterfaces, int *numPreciceInterfaces);
void PreciceInterface_CreateInterface(PreciceInterface * interface, struct CalculiXData ccx, const CoupledSurfaceConfig config);
void PreciceInterface_ConfigureFaceCentersMesh(PreciceInterface * interface, struct CalculiXData ccx);
void PreciceInterface_ConfigureNodesMesh(PreciceInterface * interface, struct CalculiXData ccx);
void PreciceInterface_ConfigureTetraFaces(PreciceInterface * interface, struct CalculiXData ccx);
void PreciceInterface_ConfigureHeatTransferData(PreciceInterface * interface, struct CalculiXData ccx);
void PreciceInterface_AdjustSolverTimestep(double precice_dt, double tper, double * dtheta, double * solver_dt);
void PreciceInterface_WriteIterationCheckpoint(struct CalculiXData * ccx, double * v);
void PreciceInterface_ReadIterationCheckpoint(CalculiXData * ccx, double * v);


#endif // PRECICEINTERFACE_H
