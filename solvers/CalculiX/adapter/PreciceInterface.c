#include <stdlib.h>
#include "PreciceInterface.h"
#include "ConfigReader.h"
#include "precice/adapters/c/SolverInterfaceC.h"


void PreciceInterface_Setup( char * configFilename, char * participantName, struct CalculiXData ccx, struct PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces )
{
	char * preciceConfigFilename;

	InterfaceConfig * interfaces;

	ConfigReader_Read( "config.yml", participantName, &preciceConfigFilename, &interfaces, numPreciceInterfaces );
	precicec_createSolverInterface( participantName, preciceConfigFilename, 0, 1 );

	*preciceInterfaces = (struct PreciceInterface**) malloc( *numPreciceInterfaces * sizeof( struct PreciceInterface* ) ); // TODO: free memory

	int i;

	for( i = 0 ; i < *numPreciceInterfaces ; i++ )
	{
		( *preciceInterfaces )[i] = malloc( sizeof( struct PreciceInterface ) );
		PreciceInterface_CreateInterface( ( *preciceInterfaces )[i], ccx, &interfaces[i] );
	}
}

void PreciceInterface_CreateInterface( struct PreciceInterface * interface, struct CalculiXData ccx, InterfaceConfig * config )
{

	interface->name = config->patchName;

	// Nodes mesh
	interface->nodesMeshName = config->nodesMeshName;
	PreciceInterface_ConfigureNodesMesh( interface, ccx );

	// Face centers mesh
	interface->faceCentersMeshName = config->facesMeshName;
	PreciceInterface_ConfigureFaceCentersMesh( interface, ccx );

	// Triangles of the nodes mesh (needs to be called after the face centers mesh is configured!)
	PreciceInterface_ConfigureTetraFaces( interface, ccx );

	PreciceInterface_ConfigureHeatTransferData( interface, ccx, config );

}

void PreciceInterface_ConfigureFaceCentersMesh( struct PreciceInterface * interface, struct CalculiXData ccx )
{

	char * faceSetName = toFaceSetName( interface->name );
	interface->faceSetID = getSetID( faceSetName, ccx.set, ccx.nset );
	interface->numElements = getNumSetElements( interface->faceSetID, ccx.istartset, ccx.iendset );

	interface->elementIDs = malloc( interface->numElements * sizeof( ITG ) );
	interface->faceIDs = malloc( interface->numElements * sizeof( ITG ) );
	getSurfaceElementsAndFaces( interface->faceSetID, ccx.ialset, ccx.istartset, ccx.iendset, interface->elementIDs, interface->faceIDs );

	interface->faceCenterCoordinates = malloc( interface->numElements * 3 * sizeof( double ) );
	getTetraFaceCenters( interface->elementIDs, interface->faceIDs, interface->numElements, ccx.kon, ccx.ipkon, ccx.co, interface->faceCenterCoordinates );

	interface->faceCentersMeshID = precicec_getMeshID( interface->faceCentersMeshName );
	interface->preciceFaceCenterIDs = malloc( interface->numElements * sizeof( int ) );
	precicec_setMeshVertices( interface->faceCentersMeshID, interface->numElements, interface->faceCenterCoordinates, interface->preciceFaceCenterIDs );

}

void PreciceInterface_ConfigureNodesMesh( struct PreciceInterface * interface, struct CalculiXData ccx )
{

	char * nodeSetName = toNodeSetName( interface->name );
	interface->nodeSetID = getSetID( nodeSetName, ccx.set, ccx.nset );
	interface->numNodes = getNumSetElements( interface->nodeSetID, ccx.istartset, ccx.iendset );
	interface->nodeIDs = &ccx.ialset[ccx.istartset[interface->nodeSetID] - 1]; // TODO: make a copy

	interface->nodeCoordinates = malloc( interface->numNodes * 3 * sizeof( double ) );
	getNodeCoordinates( interface->nodeIDs, interface->numNodes, ccx.co, interface->nodeCoordinates );

	if( interface->nodesMeshName != NULL )
	{
		interface->nodesMeshID = precicec_getMeshID( interface->nodesMeshName );
		interface->preciceNodeIDs = malloc( interface->numNodes * sizeof( int ) );
		precicec_setMeshVertices( interface->nodesMeshID, interface->numNodes, interface->nodeCoordinates, interface->preciceNodeIDs );
	}

}

void PreciceInterface_ConfigureTetraFaces( struct PreciceInterface * interface, struct CalculiXData ccx )
{
	int i;

	if( interface->nodesMeshName != NULL )
	{
		interface->triangles = malloc( interface->numElements * 3 * sizeof( ITG ) );
		getTetraFaceNodes( interface->elementIDs, interface->faceIDs,  interface->nodeIDs, interface->numElements, interface->numNodes, ccx.kon, ccx.ipkon, interface->triangles );

		for( i = 0 ; i < interface->numElements ; i++ )
		{
			precicec_setMeshTriangleWithEdges( interface->nodesMeshID, interface->triangles[3*i], interface->triangles[3*i+1], interface->triangles[3*i+2] );
		}
	}
}

void PreciceInterface_ConfigureHeatTransferData( struct PreciceInterface * interface, struct CalculiXData ccx, InterfaceConfig * config )
{

	interface->nodeData = malloc( interface->numNodes * sizeof( double ) );
	interface->faceCenterData = malloc( interface->numElements * sizeof( double ) );

	int i;

	for( i = 0 ; i < config->numReadData ; i++ )
	{
		if( strcmp( config->readDataNames[i], "Temperature" ) == 0 )
		{
			interface->readData = TEMPERATURE;
			interface->xbounIndices = malloc( interface->numNodes * sizeof( int ) );
			interface->temperatureDataID = precicec_getDataID( "Temperature", interface->nodesMeshID );
			getXbounIndices( interface->nodeIDs, interface->numNodes, ccx.nboun, ccx.ikboun, ccx.ilboun, interface->xbounIndices );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
			break;
		}
		else if ( strcmp( config->readDataNames[i], "Heat-Flux" ) == 0 )
		{
			interface->readData = HEAT_FLUX;
			interface->xloadIndices = malloc( interface->numElements * sizeof( int ) );
			getXloadIndices( "DFLUX", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices );
			interface->fluxDataID = precicec_getDataID( "Heat-Flux", interface->faceCentersMeshID );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
			break;
		}
		else if ( strcmp1( config->readDataNames[i], "Sink-Temperature-" ) == 0 )
		{
			interface->readData = KDELTA_TEMPERATURE;
			interface->xloadIndices = malloc( interface->numElements * sizeof( int ) );
			getXloadIndices( "FILM", interface->elementIDs, interface->faceIDs, interface->numElements, ccx.nload, ccx.nelemload, ccx.sideload, interface->xloadIndices );
			interface->kDeltaTemperatureReadDataID = precicec_getDataID( config->readDataNames[i], interface->faceCentersMeshID );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
		}
		else if ( strcmp1( config->readDataNames[i], "Heat-Transfer-Coefficient-" ) == 0 )
		{
			interface->kDeltaReadDataID = precicec_getDataID( config->readDataNames[i], interface->faceCentersMeshID );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
		}
		else
		{
			printf( "ERROR: Read data '%s' does not exist!\n", config->readDataNames[i] );
			exit( 1 );
		}
	}

	for( i = 0 ; i < config->numWriteData ; i++ )
	{
		if( strcmp( config->writeDataNames[i], "Temperature" ) == 0 )
		{
			interface->writeData = TEMPERATURE;
			interface->temperatureDataID = precicec_getDataID( "Temperature", interface->nodesMeshID );
			printf( "Write data '%s' found.\n", config->writeDataNames[i] );
			break;
		}
		else if ( strcmp( config->writeDataNames[i], "Heat-Flux" ) == 0 )
		{
			interface->writeData = HEAT_FLUX;
			interface->fluxDataID = precicec_getDataID( "Heat-Flux", interface->faceCentersMeshID );
			printf( "Write data '%s' found.\n", config->writeDataNames[i] );
			break;
		}
		else if ( strcmp1( config->writeDataNames[i], "Sink-Temperature-" ) == 0 )
		{
			interface->writeData = KDELTA_TEMPERATURE;
			interface->kDeltaTemperatureWriteDataID = precicec_getDataID( config->writeDataNames[i], interface->faceCentersMeshID );
			printf( "Write data '%s' found.\n", config->writeDataNames[i] );
		}
		else if ( strcmp1( config->writeDataNames[i], "Heat-Transfer-Coefficient-" ) == 0 )
		{
			interface->kDeltaWriteDataID = precicec_getDataID( config->writeDataNames[i], interface->faceCentersMeshID );
			printf( "Write data '%s' found.\n", config->writeDataNames[i] );
		}
		else
		{
			printf( "ERROR: Write data '%s' does not exist!\n", config->writeDataNames[i] );
			exit( 1 );
		}
	}
}

void PreciceInterface_WriteIterationCheckpoint( CalculiXData * ccx, double * v )
{

	// Save time
	ccx->coupling_init_theta = *( ccx->theta );

	// Save step size
	ccx->coupling_init_dtheta = *( ccx->dtheta );

	// Save solution vector v
	memcpy( ccx->coupling_init_v, v, sizeof( double ) * ccx->mt * ccx->nk );

}

void PreciceInterface_ReadIterationCheckpoint( CalculiXData * ccx, double * v )
{

	// Reload time
	*( ccx->theta ) = ccx->coupling_init_theta;

	// Reload step size
	*( ccx->dtheta ) = ccx->coupling_init_dtheta;

	// Reload solution vector v
	memcpy( v, ccx->coupling_init_v, sizeof( double ) * ccx->mt * ccx->nk );

}

void PreciceInterface_AdjustSolverTimestep( CalculiXData ccx, double precice_dt, double * solver_dt )
{
	if( isSteadyStateSimulation( ccx.nmethod ) )
	{
        printf("Adjusting time step for steady-state step\n");
        
		// For steady-state simulations, we will always compute the converged steady-state solution in one coupling step
		*ccx.theta = 0;
		*ccx.tper = 1;
		*ccx.dtheta = 1;

		// Do not subcycle! Set the solver time step to be the same as the coupling time step
		*solver_dt = precice_dt;
	}
	else
	{
        printf("Adjusting time step for transient step\n");
		printf( "precice_dt dtheta = %f, dtheta = %f, solver_dt = %f\n", precice_dt / *ccx.tper, *ccx.dtheta, fmin( precice_dt, *ccx.dtheta * *ccx.tper ) );

		// Compute the normalized time step used by CalculiX
		*ccx.dtheta = fmin( precice_dt / *ccx.tper, *ccx.dtheta );

		// Compute the non-normalized time step used by preCICE
		*solver_dt = ( *ccx.dtheta ) * ( *ccx.tper );
	}
}

void PreciceInterface_DeallocateAll()
{
	// Deallocate the data structures of each interface
}

