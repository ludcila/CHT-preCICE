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

#include <mpi.h>
#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "turbulentTransportModel.H"
#include "radiationModel.H"
#include "fvIOoptionList.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"
#include "precice/SolverInterface.hpp"
#include <sstream>
#include "adapter/ConfigReader.h"
#include "adapter/Coupler.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantBoussinesqPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantBoussinesqPimpleHeatFluxBoundaryValues.h"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addOption("precice-participant", "string", "name of preCICE participant");
    argList::addOption("precice-config", "string", "name of preCICE config file");
    
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    pimpleControl pimple(mesh);

    #include "createFields.H"
    #include "createIncompressibleRadiationModel.H"
    #include "createMRF.H"
    #include "createFvOptions.H"
    #include "initContinuityErrs.H"
    #include "createTimeControls.H"
    #include "CourantNo.H"
    #include "setInitialDeltaT.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    std::string participantName = args.optionFound("precice-participant") ? args.optionRead<string>("precice-participant") : "Fluid";
    std::string preciceConfig = args.optionFound("precice-config") ? args.optionRead<string>("precice-config") : "config.yml";
    bool checkpointingEnabled = ! args.optionFound("disable-checkpointing");
    ofcoupler::ConfigReader config(preciceConfig, participantName);

    int mpiUsed, rank = 0, size = 1;
    MPI_Initialized(&mpiUsed);
    if(mpiUsed) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    precice::SolverInterface precice(participantName, rank, size);
    precice.configure(config.preciceConfigFilename());
    ofcoupler::Coupler coupler(precice, mesh, "buoyantBoussinesqPimpleFoam");

    for(int i = 0; i < config.interfaces().size(); i++) {
        
        ofcoupler::CoupledSurface & coupledSurface = coupler.addNewCoupledSurface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);

        for(int j = 0; j < config.interfaces().at(i).writeData.size(); j++) {
            std::string dataName = config.interfaces().at(i).writeData.at(j);
            std::cout << dataName << std::endl;
            if(dataName.compare("Temperature") == 0) {
                ofcoupler::TemperatureBoundaryValues * bw = new ofcoupler::TemperatureBoundaryValues(T);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.compare("Heat-Flux") == 0) {
                ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryValues * bw = new ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryValues(T, turbulence, alphat, Pr.value(), rho.value(), Cp.value());
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else {
                std::cout << "Error: " << dataName << " does not exist." << std::endl;
                return 1;
            }
        }
        
        for(int j = 0; j < config.interfaces().at(i).readData.size(); j++) {
            std::string dataName = config.interfaces().at(i).readData.at(j);
            std::cout << dataName << std::endl;
            if(dataName.compare("Temperature") == 0) {
                ofcoupler::TemperatureBoundaryCondition * br = new ofcoupler::TemperatureBoundaryCondition(T);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.compare("Heat-Flux") == 0) {
                ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition * br = new ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition(T, turbulence, alphat, Pr.value(), rho.value(), Cp.value());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else {
                std::cout << "Error: " << dataName << " does not exist." << std::endl;
                return 1;
            }
        }
        
    }


    double precice_dt = precice.initialize();
    precice.initializeData();

    const std::string& coric = precice::constants::actionReadIterationCheckpoint();
    const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();

    Info<< "\nStarting time loop\n" << endl;

    runTime++;

    while(precice.isCouplingOngoing())
    {

        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "createTimeControls.H"
        #include "CourantNo.H"
        #include "setDeltaT.H"


        /* =========================== preCICE read data =========================== */

        if(precice.isActionRequired(cowic)){
            precice.fulfilledAction(cowic);
        }

        coupler.receiveCouplingData();

        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            #include "UEqn.H"
            #include "TEqn.H"

            // --- Pressure corrector loop
            while (pimple.correct())
            {
                #include "pEqn.H"
            }

            if (pimple.turbCorr())
            {
                laminarTransport.correct();
                turbulence->correct();
            }
        }


        /* =========================== preCICE write data =========================== */


        coupler.sendCouplingData();

        precice_dt = precice.advance(precice_dt);

        if(precice.isActionRequired(coric)){
            precice.fulfilledAction(coric);
        } else {

            runTime.write();

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

            runTime++;
        }

    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
