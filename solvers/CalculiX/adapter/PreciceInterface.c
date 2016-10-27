#include <stdlib.h>
#include "PreciceInterface.h"
#include "ConfigReader.h"
#include "precice/adapters/c/SolverInterfaceC.h"

void PreciceInterface_Initialize( SimulationData * sim )
{
	sim->precice_dt = malloc( sizeof( int ) );
	*sim->precice_dt = precicec_initialize();
	NNEW( sim->coupling_init_v, double, sim->mt * sim->nk );
}

void PreciceInterface_DoInitialExchange()
{
	printf( "Initializing coupling data\n" );
	precicec_initialize_data();
}

void PreciceInterface_Advance( SimulationData sim )
{
	*sim.precice_dt = precicec_advance( *sim.solver_dt );
}

bool PreciceInterface_IsCouplingOngoing()
{
	return precicec_isCouplingOngoing();
}

bool PreciceInterface_IsReadCheckpointRequired()
{
	return precicec_isActionRequired( "read-iteration-checkpoint" );
}

bool PreciceInterface_IsWriteCheckpointRequired()
{
	return precicec_isActionRequired( "write-iteration-checkpoint" );
}

void PreciceInterface_FulfilledReadCheckpoint()
{
	precicec_fulfilledAction( "read-iteration-checkpoint" );
}

void PreciceInterface_FulfilledWriteCheckpoint()
{
	precicec_fulfilledAction( "write-iteration-checkpoint" );
}

void PreciceInterface_Setup( char * configFilename, char * participantName, struct SimulationData sim, struct PreciceInterface *** preciceInterfaces, int * numPreciceInterfaces )
{
	char * preciceConfigFilename;

	InterfaceConfig * interfaces;

	printf( "Setting up preCICE participant %s, using config file: %s\n", participantName, configFilename );

	ConfigReader_Read( "config.yml", participantName, &preciceConfigFilename, &interfaces, numPreciceInterfaces );
	precicec_createSolverInterface( participantName, preciceConfigFilename, 0, 1 );

	*preciceInterfaces = (struct PreciceInterface**) malloc( *numPreciceInterfaces * sizeof( struct PreciceInterface* ) ); // TODO: free memory

	int i;

	for( i = 0 ; i < *numPreciceInterfaces ; i++ )
	{
		( *preciceInterfaces )[i] = malloc( sizeof( struct PreciceInterface ) );
		PreciceInterface_CreateInterface( ( *preciceInterfaces )[i], sim, &interfaces[i] );
	}
}

void PreciceInterface_CreateInterface( struct PreciceInterface * interface, struct SimulationData sim, InterfaceConfig * config )
{

	interface->name = config->patchName;

	// Nodes mesh
	interface->nodesMeshName = config->nodesMeshName;
	PreciceInterface_ConfigureNodesMesh( interface, sim );

	// Face centers mesh
	interface->faceCentersMeshName = config->facesMeshName;
	PreciceInterface_ConfigureFaceCentersMesh( interface, sim );

	// Triangles of the nodes mesh (needs to be called after the face centers mesh is configured!)
	PreciceInterface_ConfigureTetraFaces( interface, sim );

	PreciceInterface_ConfigureHeatTransferData( interface, sim, config );

}

void PreciceInterface_ConfigureFaceCentersMesh( struct PreciceInterface * interface, struct SimulationData sim )
{

	char * faceSetName = toFaceSetName( interface->name );
	interface->faceSetID = getSetID( faceSetName, sim.set, sim.nset );
	interface->numElements = getNumSetElements( interface->faceSetID, sim.istartset, sim.iendset );

	interface->elementIDs = malloc( interface->numElements * sizeof( ITG ) );
	interface->faceIDs = malloc( interface->numElements * sizeof( ITG ) );
	getSurfaceElementsAndFaces( interface->faceSetID, sim.ialset, sim.istartset, sim.iendset, interface->elementIDs, interface->faceIDs );

	interface->faceCenterCoordinates = malloc( interface->numElements * 3 * sizeof( double ) );
	getTetraFaceCenters( interface->elementIDs, interface->faceIDs, interface->numElements, sim.kon, sim.ipkon, sim.co, interface->faceCenterCoordinates );

	interface->faceCentersMeshID = precicec_getMeshID( interface->faceCentersMeshName );
	interface->preciceFaceCenterIDs = malloc( interface->numElements * sizeof( int ) );
	precicec_setMeshVertices( interface->faceCentersMeshID, interface->numElements, interface->faceCenterCoordinates, interface->preciceFaceCenterIDs );

}

void PreciceInterface_ConfigureNodesMesh( struct PreciceInterface * interface, struct SimulationData sim )
{

	char * nodeSetName = toNodeSetName( interface->name );
	interface->nodeSetID = getSetID( nodeSetName, sim.set, sim.nset );
	interface->numNodes = getNumSetElements( interface->nodeSetID, sim.istartset, sim.iendset );
	interface->nodeIDs = &sim.ialset[sim.istartset[interface->nodeSetID] - 1]; // TODO: make a copy

	interface->nodeCoordinates = malloc( interface->numNodes * 3 * sizeof( double ) );
	getNodeCoordinates( interface->nodeIDs, interface->numNodes, sim.co, interface->nodeCoordinates );

	if( interface->nodesMeshName != NULL )
	{
		interface->nodesMeshID = precicec_getMeshID( interface->nodesMeshName );
		interface->preciceNodeIDs = malloc( interface->numNodes * sizeof( int ) );
		precicec_setMeshVertices( interface->nodesMeshID, interface->numNodes, interface->nodeCoordinates, interface->preciceNodeIDs );
	}

}

void PreciceInterface_ConfigureTetraFaces( struct PreciceInterface * interface, struct SimulationData sim )
{
	int i;

	if( interface->nodesMeshName != NULL )
	{
		interface->triangles = malloc( interface->numElements * 3 * sizeof( ITG ) );
		getTetraFaceNodes( interface->elementIDs, interface->faceIDs,  interface->nodeIDs, interface->numElements, interface->numNodes, sim.kon, sim.ipkon, interface->triangles );

		for( i = 0 ; i < interface->numElements ; i++ )
		{
			precicec_setMeshTriangleWithEdges( interface->nodesMeshID, interface->triangles[3*i], interface->triangles[3*i+1], interface->triangles[3*i+2] );
		}
	}
}

void PreciceInterface_ConfigureHeatTransferData( struct PreciceInterface * interface, struct SimulationData sim, InterfaceConfig * config )
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
			getXbounIndices( interface->nodeIDs, interface->numNodes, sim.nboun, sim.ikboun, sim.ilboun, interface->xbounIndices );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
			break;
		}
		else if ( strcmp( config->readDataNames[i], "Heat-Flux" ) == 0 )
		{
			interface->readData = HEAT_FLUX;
			interface->xloadIndices = malloc( interface->numElements * sizeof( int ) );
			getXloadIndices( "DFLUX", interface->elementIDs, interface->faceIDs, interface->numElements, sim.nload, sim.nelemload, sim.sideload, interface->xloadIndices );
			interface->fluxDataID = precicec_getDataID( "Heat-Flux", interface->faceCentersMeshID );
			printf( "Read data '%s' found.\n", config->readDataNames[i] );
			break;
		}
		else if ( strcmp1( config->readDataNames[i], "Sink-Temperature-" ) == 0 )
		{
			interface->readData = KDELTA_TEMPERATURE;
			interface->xloadIndices = malloc( interface->numElements * sizeof( int ) );
			getXloadIndices( "FILM", interface->elementIDs, interface->faceIDs, interface->numElements, sim.nload, sim.nelemload, sim.sideload, interface->xloadIndices );
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

void PreciceInterface_WriteIterationCheckpoint( SimulationData * sim, double * v )
{

	// Save time
	sim->coupling_init_theta = *( sim->theta );

	// Save step size
	sim->coupling_init_dtheta = *( sim->dtheta );

	// Save solution vector v
	memcpy( sim->coupling_init_v, v, sizeof( double ) * sim->mt * sim->nk );

}

void PreciceInterface_ReadIterationCheckpoint( SimulationData * sim, double * v )
{

	// Reload time
	*( sim->theta ) = sim->coupling_init_theta;

	// Reload step size
	*( sim->dtheta ) = sim->coupling_init_dtheta;

	// Reload solution vector v
	memcpy( v, sim->coupling_init_v, sizeof( double ) * sim->mt * sim->nk );

}

void PreciceInterface_AdjustSolverTimestep( SimulationData sim )
{
	if( isSteadyStateSimulation( sim.nmethod ) )
	{
		printf( "Adjusting time step for steady-state step\n" );

		// For steady-state simulations, we will always compute the converged steady-state solution in one coupling step
		*sim.theta = 0;
		*sim.tper = 1;
		*sim.dtheta = 1;

		// Do not subcycle! Set the solver time step to be the same as the coupling time step
		*sim.solver_dt = *sim.precice_dt;
	}
	else
	{
		printf( "Adjusting time step for transient step\n" );
		printf( "precice_dt dtheta = %f, dtheta = %f, solver_dt = %f\n", *sim.precice_dt / *sim.tper, *sim.dtheta, fmin( *sim.precice_dt, *sim.dtheta * *sim.tper ) );

		// Compute the normalized time step used by CalculiX
		*sim.dtheta = fmin( *sim.precice_dt / *sim.tper, *sim.dtheta );

		// Compute the non-normalized time step used by preCICE
		*sim.solver_dt = ( *sim.dtheta ) * ( *sim.tper );
	}
}

void PreciceInterface_ReadCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces )
{
	int i;

	if( precicec_isReadDataAvailable() )
	{
		for( i = 0 ; i < numInterfaces ; i++ )
		{
			switch( preciceInterfaces[i]->readData )
			{
			case TEMPERATURE:
				// Read and set temperature BC
				precicec_readBlockScalarData( preciceInterfaces[i]->temperatureDataID, preciceInterfaces[i]->numNodes, preciceInterfaces[i]->preciceNodeIDs, preciceInterfaces[i]->nodeData );
				setNodeTemperatures( preciceInterfaces[i]->nodeData, preciceInterfaces[i]->numNodes, preciceInterfaces[i]->xbounIndices, sim.xboun );
				break;
			case HEAT_FLUX:
				// Read and set heat flux BC
				precicec_readBlockScalarData( preciceInterfaces[i]->fluxDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, preciceInterfaces[i]->faceCenterData );
				setFaceFluxes( preciceInterfaces[i]->faceCenterData, preciceInterfaces[i]->numElements, preciceInterfaces[i]->xloadIndices, sim.xload );
				break;
			case KDELTA_TEMPERATURE:
				// Read and set sink temperature in convective film BC
				precicec_readBlockScalarData( preciceInterfaces[i]->kDeltaTemperatureReadDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, preciceInterfaces[i]->faceCenterData );
				setFaceSinkTemperatures( preciceInterfaces[i]->faceCenterData, preciceInterfaces[i]->numElements, preciceInterfaces[i]->xloadIndices, sim.xload );
				// Read and set heat transfer coefficient in convective film BC
				precicec_readBlockScalarData( preciceInterfaces[i]->kDeltaReadDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, preciceInterfaces[i]->faceCenterData );
				setFaceHeatTransferCoefficients( preciceInterfaces[i]->faceCenterData, preciceInterfaces[i]->numElements, preciceInterfaces[i]->xloadIndices, sim.xload );
				break;
			}
		}
	}
}

void PreciceInterface_WriteCouplingData( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces )
{

	int i;
	int iset;

	if( precicec_isWriteDataRequired( *sim.solver_dt ) || precicec_isActionRequired( "write-initial-data" ) )
	{
		for( i = 0 ; i < numInterfaces ; i++ )
		{
			switch( preciceInterfaces[i]->writeData )
			{
			case TEMPERATURE:
				getNodeTemperatures( preciceInterfaces[i]->nodeIDs, preciceInterfaces[i]->numNodes, sim.vold, sim.mt, preciceInterfaces[i]->nodeData );
				precicec_writeBlockScalarData( preciceInterfaces[i]->temperatureDataID, preciceInterfaces[i]->numNodes, preciceInterfaces[i]->preciceNodeIDs, preciceInterfaces[i]->nodeData );
				break;
			case HEAT_FLUX:
				iset = preciceInterfaces[i]->faceSetID + 1; // Adjust index before calling Fortran function
				FORTRAN( getflux, ( sim.co,
				                    sim.ntmat_,
				                    sim.vold,
				                    sim.cocon,
				                    sim.ncocon,
				                    &iset,
				                    sim.istartset,
				                    sim.iendset,
				                    sim.ipkon,
				                    *sim.lakon,
				                    sim.kon,
				                    sim.ialset,
				                    sim.ielmat,
				                    sim.mi,
				                    preciceInterfaces[i]->faceCenterData
				                    )
				         );
				precicec_writeBlockScalarData( preciceInterfaces[i]->fluxDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, preciceInterfaces[i]->faceCenterData );
				break;
			case KDELTA_TEMPERATURE:
				iset = preciceInterfaces[i]->faceSetID + 1; // Adjust index before calling Fortran function
				double * myKDelta = malloc( preciceInterfaces[i]->numElements * sizeof( double ) );
				double * T = malloc( preciceInterfaces[i]->numElements * sizeof( double ) );
				FORTRAN( getkdeltatemp, ( sim.co,
				                          sim.ntmat_,
				                          sim.vold,
				                          sim.cocon,
				                          sim.ncocon,
				                          &iset,
				                          sim.istartset,
				                          sim.iendset,
				                          sim.ipkon,
				                          *sim.lakon,
				                          sim.kon,
				                          sim.ialset,
				                          sim.ielmat,
				                          sim.mi,
				                          myKDelta,
				                          T
				                          )
				         );
				precicec_writeBlockScalarData( preciceInterfaces[i]->kDeltaWriteDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, myKDelta );
				precicec_writeBlockScalarData( preciceInterfaces[i]->kDeltaTemperatureWriteDataID, preciceInterfaces[i]->numElements, preciceInterfaces[i]->preciceFaceCenterIDs, T );
				free( myKDelta );
				free( T );
				break;

			}
		}

		if( precicec_isActionRequired( "write-initial-data" ) )
		{
			printf( "Initial data written\n" );
			precicec_fulfilledAction( "write-initial-data" );
		}
	}
}

void PreciceInterface_FreeData( PreciceInterface * preciceInterface )
{
	free( preciceInterface->elementIDs );
	free( preciceInterface->faceIDs );
	free( preciceInterface->faceCenterCoordinates );
	free( preciceInterface->preciceFaceCenterIDs );
	free( preciceInterface->nodeCoordinates );

	if( preciceInterface->preciceNodeIDs != NULL )
		free( preciceInterface->preciceNodeIDs );

	if( preciceInterface->triangles != NULL )
		free( preciceInterface->triangles );

	free( preciceInterface->nodeData );
	free( preciceInterface->faceCenterData );

	if( preciceInterface->xbounIndices != NULL )
		free( preciceInterface->xbounIndices );

	if( preciceInterface->xloadIndices != NULL )
		free( preciceInterface->xloadIndices );

}

void PreciceInterface_FreeAll( SimulationData sim, PreciceInterface ** preciceInterfaces, int numInterfaces )
{
	int i;

	free( sim.precice_dt );
	free( sim.coupling_init_v );

	for( i = 0 ; i < numInterfaces ; i++ )
	{
		PreciceInterface_FreeData( preciceInterfaces[i] );
		free( preciceInterfaces[i] );
	}
}

