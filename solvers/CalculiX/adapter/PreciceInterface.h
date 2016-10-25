#ifndef PRECICEINTERFACE_H
#define PRECICEINTERFACE_H

#include "ConfigReader.h"
#include "CCXHelpers.h"
#include "Helpers.h"

enum CouplingDataType {TEMPERATURE, HEAT_FLUX, SINK_TEMPERATURE, KDELTA_TEMPERATURE};

/*
 * CalculiXData: Structure with all the CalculiX variables 
 * that need to be accessed by the adapter in order to do the coupling
 */
typedef struct CalculiXData {
	ITG * ialset;
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
	double mt;
	double * theta;
	double * dtheta;
	double * coupling_init_v;
	double coupling_init_theta;
	double coupling_init_dtheta;
} CalculiXData;

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
 * @brief PreciceInterface_Setup
 * @param configFilename
 * @param participantName
 * @param ccx
 * @param preciceInterfaces
 * @param numPreciceInterfaces
 */
void PreciceInterface_Setup( char * configFilename, char * participantName, struct CalculiXData ccx, PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces );

/**
 * @brief PreciceInterface_CreateInterface
 * @param interface
 * @param ccx
 * @param config
 */
void PreciceInterface_CreateInterface( PreciceInterface * interface, struct CalculiXData ccx, InterfaceConfig * config );

/**
 * @brief Configures the face centers mesh and calls setMeshVertices on preCICE
 * @param interface
 * @param ccx
 */
void PreciceInterface_ConfigureFaceCentersMesh( PreciceInterface * interface, struct CalculiXData ccx );

/**
 * @brief PreciceInterface_ConfigureNodesMesh
 * @param interface
 * @param ccx: Structure with CalculiX data
 */
void PreciceInterface_ConfigureNodesMesh( PreciceInterface * interface, struct CalculiXData ccx );

/**
 * @brief PreciceInterface_ConfigureTetraFaces
 * @param interface
 * @param ccx
 */
void PreciceInterface_ConfigureTetraFaces( PreciceInterface * interface, struct CalculiXData ccx );

/**
 * @brief PreciceInterface_ConfigureHeatTransferData
 * @param interface
 * @param ccx
 * @param config
 */
void PreciceInterface_ConfigureHeatTransferData( PreciceInterface * interface, struct CalculiXData ccx, InterfaceConfig * config );

/**
 * @brief PreciceInterface_AdjustSolverTimestep
 * @param precice_dt: Maximum time step size that can be used for the coupling
 * @param tper: CalculiX variable for the total simulation time
 * @param dtheta: CalculiX variable for the step size, normalized with respect to the total time tper
 * @param solver_dt: Actual step size (dtheta * tper), used by preCICE
 */
void PreciceInterface_AdjustSolverTimestep( double precice_dt, double tper, double * dtheta, double * solver_dt );

/**
 * @brief PreciceInterface_WriteIterationCheckpoint
 * @param ccx: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values 
 */
void PreciceInterface_WriteIterationCheckpoint( struct CalculiXData * ccx, double * v );

/**
 * @brief PreciceInterface_ReadIterationCheckpoint
 * @param ccx: Structure with CalculiX data
 * @param v: CalculiX array with the temperature values
 */
void PreciceInterface_ReadIterationCheckpoint( CalculiXData * ccx, double * v );


#endif // PRECICEINTERFACE_H
