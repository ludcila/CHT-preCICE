/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
     \\/     M anipulation  |
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
    buoyantPimpleFoam

Description
    Transient solver for buoyant, turbulent flow of compressible fluids for
    ventilation and heat-transfer.

    Turbulence is modelled using a run-time selectable compressible RAS or
    LES model.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "radiationModel.H"
#include "fvIOoptionList.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"

#include "adapter/BuoyantPimpleFoamAdapter.h"

bool isTurbulenceUsed( Foam::fvMesh & mesh, Foam::Time & runTime )
{
	const word turbulenceModel
	(
	        IOdictionary
	        (
	                IOobject
	                (
	                        "turbulenceProperties",
	                        runTime.constant(),
	                        mesh,
	                        IOobject::MUST_READ_IF_MODIFIED,
	                        IOobject::NO_WRITE
	                )
	        ).lookup( "simulationType" )
	);
	return ( turbulenceModel != "laminar" );
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{

    /* Adapter: Add command line arguments */    
    argList::addOption( "precice-participant", "string", "name of preCICE participant" );
    argList::addOption( "config-file", "string", "name of YAML config file" );
    argList::addBoolOption( "disable-checkpointing", "disable checkpointing" );

    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    pimpleControl pimple(mesh);

    #include "createFields.H"
    #include "createMRF.H"
    #include "createFvOptions.H"
    #include "createRadiationModel.H"
    #include "initContinuityErrs.H"
    #include "createTimeControls.H"
    #include "compressibleCourantNo.H"
    #include "setInitialDeltaT.H"
    
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    /* Adapter: Set parameters and create the adapter */
	bool turbulenceUsed = isTurbulenceUsed( mesh, runTime );

	std::string participantName = args.optionFound( "precice-participant" ) ?
								  args.optionRead<string>( "precice-participant" ) : "Fluid";

	std::string configFile = args.optionFound( "config-file" ) ?
							 args.optionRead<string>( "config-file" ) : "config.yml";

	bool checkpointingEnabled = !args.optionFound( "disable-checkpointing" );

	bool subcyclingEnabled = true;
	adapter::BuoyantPimpleFoamAdapter adapter( participantName,
											   configFile,
											   mesh,
											   runTime,
											   "buoyantPimpleFoam",
											   thermo,
											   turbulence,
											   subcyclingEnabled );

    /* Adapter: Add fields for checkpointing */
	adapter.setCheckpointingEnabled( checkpointingEnabled );
	adapter.addCheckpointField( U );
	adapter.addCheckpointField( p );
	adapter.addCheckpointField( p_rgh );
	adapter.addCheckpointField( rho );
	adapter.addCheckpointField( thermo.T() );
	adapter.addCheckpointField( thermo.he() );
	adapter.addCheckpointField( thermo.p() );
	adapter.addCheckpointField( K );
	adapter.addCheckpointField( dpdt );
	adapter.addCheckpointField( phi );

	if( turbulenceUsed )
	{
		adapter.addCheckpointField( turbulence->k() () );
		adapter.addCheckpointField( turbulence->epsilon() () );
		adapter.addCheckpointField( turbulence->nut() () );
		adapter.addCheckpointField( turbulence->alphat() () );
	}

    /* Adapter: Initialize coupling */
    adapter.initialize();
       
    Info<< "\nStarting time loop\n" << endl;

    /* Adapter: Give time stepping control to the adapter */
    while (adapter.isCouplingOngoing())
    {

        #include "createTimeControls.H"
        #include "compressibleCourantNo.H"
        #include "setDeltaT.H"
        
        /* Adapter: Adjust solver time step if necessary */
        adapter.adjustSolverTimeStep();

        /* Adapter: Write checkpoint if necessary */
        if( adapter.isWriteCheckpointRequired() )
        {
            adapter.writeCheckpoint();
            adapter.fulfilledWriteCheckpoint();
        }

        runTime++;
        
        /* Adapter: Receive coupling data */
        adapter.readCouplingData();

        /* Start of original solver code */

        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "rhoEqn.H"

        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            #include "UEqn.H"
            #include "EEqn.H"

            // --- Pressure corrector loop
            while (pimple.correct())
            {
                #include "pEqn.H"
            }

            if (pimple.turbCorr())
            {
                turbulence->correct();
            }
        }

        rho = thermo.rho();

        /* End of original solver code */
        
        /* Adapter: Send coupling data and advance */
        adapter.writeCouplingData();
        adapter.advance();
    
        /* Adapter: Read checkpoint if necessary (coupling not converged) */
		if( adapter.isReadCheckpointRequired() )
		{

			adapter.readCheckpoint();

			if( turbulenceUsed && adapter.isCheckpointingEnabled() )
			{
				turbulence->alphat() ().correctBoundaryConditions();
			}
			adapter.fulfilledReadCheckpoint();

		}
		else
		{
            
            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;
		}
        
		if( adapter.isCouplingTimeStepComplete() )
		{
			runTime.write();
		}

    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
