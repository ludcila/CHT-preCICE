/*---------------------------------------------------------------------------*\
   =========                 |
\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
\\    /   O peration     |
\\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
\\\\\\\\\\\\\\\\\\\\\\\\/     M anipulation  |
   -------------------------------------------------------------------------------
   License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

   Application
    laplacianFoam

   Description
    Solves a simple Laplace equation, e.g. for thermal diffusion in a solid.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "simpleControl.H"
#include "adapter/ConfigReader.h"
#include "adapter/Adapter.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/HeatFluxBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/HeatFluxBoundaryCondition.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void addCouplingData( adapter::Adapter & adapter, std::string configFile, std::string participantName,
					  volScalarField & T, dimensionedScalar & k );

int main( int argc, char * argv[] )
{
	argList::addOption( "precice-participant",
						"string",
						"name of preCICE participant" );

	argList::addOption( "config-file",
						"string",
						"name of preCICE config file" );

	#include "setRootCase.H"

	#include "createTime.H"
	#include "createMesh.H"
	#include "createFields.H"

	simpleControl simple( mesh );

	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


	std::string participantName = args.optionFound( "precice-participant" ) ?
								  args.optionRead<string>( "precice-participant" ) : "Fluid";

	std::string configFile = args.optionFound( "config-file" ) ?
							 args.optionRead<string>( "config-file" ) : "config.yml";


	bool subcyclingEnabled = true;
	adapter::Adapter adapter( participantName, configFile, mesh, runTime, subcyclingEnabled );

	addCouplingData( adapter, configFile, participantName, T, k );

	adapter.addCheckpointField( T );
	adapter.initialize();

	Info<< "\nCalculating temperature distribution\n" << endl;

	while( adapter.isCouplingOngoing() ) {

		adapter.adjustSolverTimeStep();

		if( adapter.isWriteCheckpointRequired() )
		{
			adapter.writeCheckpoint();
			adapter.fulfilledWriteCheckpoint();
		}

		simple.loop();

		adapter.readCouplingData();

		while ( simple.correctNonOrthogonal() )
		{
			solve
			(
				fvm::ddt( T ) - fvm::laplacian( k/rho/Cp, T )
			);
		}

		adapter.writeCouplingData();
		adapter.advance();

		if( adapter.isReadCheckpointRequired() )
		{
			adapter.readCheckpoint();
			adapter.fulfilledReadCheckpoint();
		}
		else
		{

			#include "write.H"

			Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
				<< "  ClockTime = " << runTime.elapsedClockTime() << " s"
				<< nl << endl;

		}
	}

	Info<< "End\n" << endl;

	return 0;
}

void addCouplingData( adapter::Adapter & adapter, std::string configFile, std::string participantName,
					  volScalarField & T, dimensionedScalar & k )
{
	adapter::ConfigReader configReader( configFile, participantName );

	for( uint i = 0 ; i < configReader.interfaces().size() ; i++ )
	{

		adapter::Interface & coupledSurface = adapter.addNewInterface( configReader.interfaces().at( i ).meshName,
																	   configReader.interfaces().at( i ).patchNames );

		for( uint j = 0 ; j < configReader.interfaces().at( i ).writeData.size() ; j++ )
		{
			std::string dataName = configReader.interfaces().at( i ).writeData.at( j );

			if( dataName.compare( "Temperature" ) == 0 )
			{
				adapter::TemperatureBoundaryValues * bw = new adapter::TemperatureBoundaryValues( T );
				coupledSurface.addCouplingDataWriter( dataName, bw );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				adapter::HeatFluxBoundaryValues * bw = new adapter::HeatFluxBoundaryValues( T, k.value() );
				coupledSurface.addCouplingDataWriter( dataName, bw );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " is not valid";
				exit( 1 );
			}
		}

		for( uint j = 0 ; j < configReader.interfaces().at( i ).readData.size() ; j++ )
		{
			std::string dataName = configReader.interfaces().at( i ).readData.at( j );

			if( dataName.compare( "Temperature" ) == 0 )
			{
				adapter::TemperatureBoundaryCondition * br = new adapter::TemperatureBoundaryCondition( T );
				coupledSurface.addCouplingDataReader( dataName, br );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				adapter::HeatFluxBoundaryCondition * br = new adapter::HeatFluxBoundaryCondition( T, k.value() );
				coupledSurface.addCouplingDataReader( dataName, br );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " is not valid";
				exit( 1 );
			}
		}
	}
}

// ************************************************************************* //
