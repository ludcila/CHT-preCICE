/*---------------------------------------------------------------------------*\
   =========                 |
\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
\\    /   O peration     |
\\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
\\\\\\\\\\\\\\/     M anipulation  |
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
    buoyantBoussinesqPimpleFoam

   Description
    Transient solver for buoyant, turbulent flow of incompressible fluids

    Uses the Boussinesq approximation:
    \f[
        rho_{k} = 1 - beta(T - T_{ref})
    \f]

    where:
        \f$ rho_{k} \f$ = the effective (driving) kinematic density
        beta = thermal expansion coefficient [1/K]
        T = temperature [K]
        \f$ T_{ref} \f$ = reference temperature [K]

    Valid when:
    \f[
        \frac{beta(T - T_{ref})}{rho_{ref}} << 1
    \f]

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "turbulentTransportModel.H"
#include "radiationModel.H"
#include "fvIOoptionList.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"
#include "adapter/ConfigReader.h"
#include "adapter/Adapter.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantBoussinesqPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantBoussinesqPimpleHeatFluxBoundaryValues.h"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void addCouplingData( adapter::Adapter & adapter, std::string configFile, std::string participantName,
					  volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat,
					  dimensionedScalar & Pr, dimensionedScalar & rho, dimensionedScalar & Cp );

int main( int argc, char * argv[] )
{
	argList::addOption( "precice-participant",
						"string",
						"name of preCICE participant" );

	argList::addOption( "config-file",
						"string",
						"name of YAML config file" );

	#include "setRootCase.H"
	#include "createTime.H"
	#include "createMesh.H"

	pimpleControl pimple( mesh );

	#include "createFields.H"
	#include "createIncompressibleRadiationModel.H"
	#include "createMRF.H"
	#include "createFvOptions.H"
	#include "initContinuityErrs.H"
	#include "createTimeControls.H"
	#include "CourantNo.H"
	#include "setInitialDeltaT.H"

	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

	std::string participantName = args.optionFound( "precice-participant" ) ?
								  args.optionRead<string>( "precice-participant" ) : "Fluid";

	std::string configFile = args.optionFound( "config-file" ) ?
							 args.optionRead<string>( "config-file" ) : "config.yml";

	adapter::Adapter adapter( participantName, configFile, mesh, runTime );

	addCouplingData( adapter, configFile, participantName, T, turbulence, alphat, Pr, rho, Cp );
	adapter.initialize();

	Info<< "\nStarting time loop\n" << endl;

	while( adapter.isCouplingOngoing() )
	{

		Info<< "Time = " << runTime.timeName() << nl << endl;

		#include "createTimeControls.H"
		#include "CourantNo.H"
		#include "setDeltaT.H"

		adapter.adjustSolverTimeStep();

		if( adapter.isWriteCheckpointRequired() )
		{
			adapter.writeCheckpoint();
			adapter.fulfilledWriteCheckpoint();
		}

		runTime++;

		adapter.readCouplingData();

		// --- Pressure-velocity PIMPLE corrector loop
		while ( pimple.loop() )
		{
			#include "UEqn.H"
			#include "TEqn.H"

			// --- Pressure corrector loop
			while ( pimple.correct() )
			{
				#include "pEqn.H"
			}

			if ( pimple.turbCorr() )
			{
				laminarTransport.correct();
				turbulence->correct();
			}
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

			runTime.write();

			Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
				<< "  ClockTime = " << runTime.elapsedClockTime() << " s"
				<< nl << endl;
		}

	}

	Info<< "End\n" << endl;

	return 0;
}

void addCouplingData( adapter::Adapter & adapter, std::string configFile, std::string participantName,
					  volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat,
					  dimensionedScalar & Pr, dimensionedScalar & rho, dimensionedScalar & Cp )
{

	adapter::ConfigReader configReader( configFile, participantName );

	for( uint i = 0 ; i < configReader.interfaces().size() ; i++ )
	{

		adapter::Interface & interface = adapter.addNewInterface( configReader.interfaces().at( i ).meshName, configReader.interfaces().at( i ).patchNames );

		for( uint j = 0 ; j < configReader.interfaces().at( i ).writeData.size() ; j++ )
		{
			std::string dataName = configReader.interfaces().at( i ).writeData.at( j );

			if( dataName.compare( "Temperature" ) == 0 )
			{
				adapter::TemperatureBoundaryValues * bw = new adapter::TemperatureBoundaryValues( T );
				interface.addCouplingDataWriter( dataName, bw );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryValues * bw = new adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryValues( T, turbulence, alphat, Pr.value(), rho.value(), Cp.value() );
				interface.addCouplingDataWriter( dataName, bw );
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
				interface.addCouplingDataReader( dataName, br );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition * br = new adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition( T, turbulence, alphat, Pr.value(), rho.value(), Cp.value() );
				interface.addCouplingDataReader( dataName, br );
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
