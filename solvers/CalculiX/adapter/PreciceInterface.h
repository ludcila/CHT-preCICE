#ifndef PRECICEINTERFACE_H
#define PRECICEINTERFACE_H

#include <string.h>
#include "ConfigReader.h"
#include "CCXHelpers.h"

enum CouplingDataType {TEMPERATURE, HEAT_FLUX, KDELTA_TEMPERATURE};

/*
 * SimulationData: Structure with all the CalculiX variables
 * that need to be accessed by the adapter in order to do the coupling.
 * A list of variables and their meaning is available in the documentation
 * ccx_2.10.pdf (page 518)
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
 * @brief Precice_Setup
 * @param configFilename
 * @param participantName
 * @param sim
 * @param preciceInterfaces
 * @param numPreciceInterfaces
 */
void Precice_Setup( char * configFilename, char * participantName, SimulationData * sim, PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces );

/**
 * @brief Precice_Initialize
 * @param sim
 */
void Precice_Initialize( SimulationData * sim );

/**
 * @brief Precice_InitializeData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void Precice_InitializeData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numPreciceInterfaces );

/**
 * @brief PreciceInterface_AdjustSolverTimestep
 * @param sim
 */
void Precice_AdjustSolverTimestep( SimulationData sim );

/**
 * @brief Precice_Advance
 * @param sim
 */
void Precice_Advance( SimulationData sim );

/**
 * @brief Returns true if coupling is still ongoing
 * @return
 */
bool Precice_IsCouplingOngoing();

/**
 * @brief Returns true if checkpoint must be read
 * @return
 */
bool Precice_IsReadCheckpointRequired();

/**
 * @brief Returns true if checkpoint must be written
 * @return
 */
bool Precice_IsWriteCheckpointRequired();

/**
 * @brief Precice_FulfilledReadCheckpoint
 */
void Precice_FulfilledReadCheckpoint();

/**
 * @brief Precice_FulfilledWriteCheckpoint
 */
void Precice_FulfilledWriteCheckpoint();

/**
 * @brief PreciceInterface_ReadIterationCheckpoint
 * @param sim: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values
 */
void Precice_ReadIterationCheckpoint( SimulationData * sim, double * v );

/**
 * @brief PreciceInterface_WriteIterationCheckpoint
 * @param sim: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values
 */
void Precice_WriteIterationCheckpoint( SimulationData * sim, double * v );

/**
 * @brief PreciceInterface_ReadCouplingData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void Precice_ReadCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );

/**
 * @brief PreciceInterface_WriteCouplingData
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void Precice_WriteCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );

/**
 * @brief Precice_FreeAll
 * @param sim
 * @param preciceInterfaces
 * @param numInterfaces
 */
void Precice_FreeAll( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces );

/**
 * @brief PreciceInterface_Create
 * @param interface
 * @param sim
 * @param config
 */
void PreciceInterface_Create( PreciceInterface * interface, SimulationData sim, InterfaceConfig * config );

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
 * @brief PreciceInterface_FreeData
 * @param preciceInterface
 */
void PreciceInterface_FreeData( PreciceInterface * preciceInterface );


#endif // PRECICEINTERFACE_H
