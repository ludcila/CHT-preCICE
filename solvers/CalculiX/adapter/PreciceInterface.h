#ifndef PRECICEINTERFACE_H
#define PRECICEINTERFACE_H

#include <string.h>
#include "ConfigReader.h"
#include "CCXHelpers.h"

enum CouplingDataType {TEMPERATURE, HEAT_FLUX, KDELTA_TEMPERATURE};

/*
 * CalculiXData: Structure with all the CalculiX variables
 * that need to be accessed by the adapter in order to do the coupling
 */
typedef struct SimulationData {
	// CalculiX data
	ITG * ialset;
	ITG * ielmat;
	ITG * istartset;
	ITG * iendset;
	char ** lakon;
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
	ITG mt;
	double * theta;
	double * dtheta;
	double * tper;
	ITG * nmethod;
	double * xload;
	double * xboun;
	ITG * ntmat_;
	double * vold;
	double * cocon;
	ITG * ncocon;
	ITG * mi;
	// Coupling data
	double * coupling_init_v;
	double coupling_init_theta;
	double coupling_init_dtheta;
	double * precice_dt;
	double * solver_dt;
} SimulationData;

/*
 * PreciceInterface: Structure with all the information of a coupled surface
 * Includes data regarding the surface mesh(es) and the coupling data
 */
typedef struct PreciceInterface {

	char * name;

	// Interface nodes
	int numNodes;
	int * nodeIDs;
	double * nodeCoordinates;
	int nodeSetID;
	int * preciceNodeIDs;
	int nodesMeshID;
	char * nodesMeshName;

	// Interface face elements
	int numElements;
	int * elementIDs;
	int * faceIDs;
	double * faceCenterCoordinates;
	int faceSetID;
	int faceCentersMeshID;
	char * faceCentersMeshName;
	int * preciceFaceCenterIDs;
	int * triangles;

	// Arrays to store the coupling data
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

	// Indices that indicate where to apply the boundary conditions
	int * xloadIndices;
	int * xbounIndices;

	enum CouplingDataType readData;
	enum CouplingDataType writeData;

} PreciceInterface;

/**
 * @brief PreciceInterface_Initialize
 * @param sim
 */
void PreciceInterface_Initialize( SimulationData * sim );

/**
 * @brief PreciceInterface_InitializeData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void PreciceInterface_InitializeData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numPreciceInterfaces );

/**
 * @brief PreciceInterface_Advance
 * @param sim
 */
void PreciceInterface_Advance( SimulationData sim );

/**
 * @brief Returns true if coupling is still ongoing
 * @return
 */
bool PreciceInterface_IsCouplingOngoing();

/**
 * @brief Returns true if checkpoint must be read
 * @return
 */
bool PreciceInterface_IsReadCheckpointRequired();

/**
 * @brief Returns true if checkpoint must be written
 * @return
 */
bool PreciceInterface_IsWriteCheckpointRequired();

/**
 * @brief PreciceInterface_FulfilledReadCheckpoint
 */
void PreciceInterface_FulfilledReadCheckpoint();

/**
 * @brief PreciceInterface_FulfilledWriteCheckpoint
 */
void PreciceInterface_FulfilledWriteCheckpoint();

/**
 * @brief PreciceInterface_Setup
 * @param configFilename
 * @param participantName
 * @param sim
 * @param preciceInterfaces
 * @param numPreciceInterfaces
 */
void PreciceInterface_Setup( char * configFilename, char * participantName, SimulationData sim, PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces );

/**
 * @brief PreciceInterface_CreateInterface
 * @param interface
 * @param sim
 * @param config
 */
void PreciceInterface_CreateInterface( PreciceInterface * interface, SimulationData sim, InterfaceConfig * config );

/**
 * @brief Configures the face centers mesh and calls setMeshVertices on preCICE
 * @param interface
 * @param sim
 */
void PreciceInterface_ConfigureFaceCentersMesh( PreciceInterface * interface, SimulationData sim );

/**
 * @brief PreciceInterface_ConfigureNodesMesh
 * @param interface
 * @param sim: Structure with CalculiX data
 */
void PreciceInterface_ConfigureNodesMesh( PreciceInterface * interface, SimulationData sim );

/**
 * @brief PreciceInterface_ConfigureTetraFaces
 * @param interface
 * @param sim
 */
void PreciceInterface_ConfigureTetraFaces( PreciceInterface * interface, SimulationData sim );

/**
 * @brief PreciceInterface_ConfigureHeatTransferData
 * @param interface
 * @param sim
 * @param config
 */
void PreciceInterface_ConfigureHeatTransferData( PreciceInterface * interface, SimulationData sim, InterfaceConfig * config );

/**
 * @brief PreciceInterface_AdjustSolverTimestep
 * @param sim
 */
void PreciceInterface_AdjustSolverTimestep( SimulationData sim );

/**
 * @brief PreciceInterface_WriteIterationCheckpoint
 * @param sim: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values
 */
void PreciceInterface_WriteIterationCheckpoint( SimulationData * sim, double * v );

/**
 * @brief PreciceInterface_ReadIterationCheckpoint
 * @param sim: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values
 */
void PreciceInterface_ReadIterationCheckpoint( SimulationData * sim, double * v );

/**
 * @brief PreciceInterface_ReadCouplingData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void PreciceInterface_ReadCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );

/**
 * @brief PreciceInterface_WriteCouplingData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void PreciceInterface_WriteCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );

/**
 * @brief PreciceInterface_FreeData
 * @param preciceInterface
 */
void PreciceInterface_FreeData( PreciceInterface * preciceInterface );

/**
 * @brief PreciceInterface_FreeAll
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void PreciceInterface_FreeAll( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );


#endif // PRECICEINTERFACE_H
