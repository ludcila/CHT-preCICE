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
    buoyantSimpleFoam

Description
    Steady-state solver for buoyant, turbulent flow of compressible fluids,
    including radiation, for ventilation and heat-transfer.

\*---------------------------------------------------------------------------*/

#include <mpi.h>
#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "radiationModel.H"
#include "simpleControl.H"
#include "fvIOoptionList.H"
#include "fixedFluxPressureFvPatchScalarField.H"
#include "precice/SolverInterface.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include "adapter/ConfigReader.h"
#include "adapter/Adapter.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/KDeltaBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/KDeltaBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/RefTemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/RefTemperatureBoundaryCondition.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    
    argList::addOption("precice-participant", "string", "name of preCICE participant");
    argList::addOption("precice-config", "string", "name of preCICE config file");

    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    simpleControl simple(mesh);

    #include "createFields.H"
    #include "createMRF.H"
    #include "createFvOptions.H"
    #include "createRadiationModel.H"
    #include "initContinuityErrs.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    std::string participantName = args.optionFound("precice-participant") ? args.optionRead<string>("precice-participant") : "Fluid";
    std::string preciceConfig = args.optionFound("precice-config") ? args.optionRead<string>("precice-config") : "config.yml";
    adapter::ConfigReader config(preciceConfig, participantName);
    
    int mpiUsed, rank = 0, size = 1;
    MPI_Initialized(&mpiUsed);
    if(mpiUsed) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    precice::SolverInterface precice(participantName, rank, size);
    precice.configure(config.preciceConfigFilename());
    precice.configure(config.preciceConfigFilename());
    adapter::Adapter coupler(precice, mesh, "buoyantSimpleFoam");


    for(int i = 0; i < config.interfaces().size(); i++) {        
        adapter::Interface & coupledSurface = coupler.addNewInterface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);
        for(int j = 0; j < config.interfaces().at(i).writeData.size(); j++) {
            std::string dataName = config.interfaces().at(i).writeData.at(j);
            if(dataName.find("Heat-Transfer-Coefficient") == 0) {
                coupledSurface.addCouplingDataWriter(dataName, new adapter::KDeltaBoundaryValues<autoPtr<compressible::RASModel> >(turbulence));
            } else if(dataName.find("Sink-Temperature") == 0) {
                coupledSurface.addCouplingDataWriter(dataName, new adapter::RefTemperatureBoundaryValues(thermo.T()));
            } else {
                std::cout << "Error: " << dataName << " is not valid" << std::endl;
                return 1;
            }
        }
        for(int j = 0; j < config.interfaces().at(i).readData.size(); j++) {
            std::string dataName = config.interfaces().at(i).readData.at(j);
            if(dataName.find("Heat-Transfer-Coefficient") == 0) {
                coupledSurface.addCouplingDataReader(dataName, new adapter::KDeltaBoundaryCondition<autoPtr<compressible::RASModel> >(thermo.T(), turbulence));
            } else if(dataName.find("Sink-Temperature") == 0) {
                coupledSurface.addCouplingDataReader(dataName, new adapter::RefTemperatureBoundaryCondition(thermo.T()));
            } else {
                std::cout << "Error: " << dataName << " is not valid" << std::endl;
                return 1;
            }
            
        }  
    }
    
    const std::string& coric = precice::constants::actionReadIterationCheckpoint();
    const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();

    double preciceDt = precice.initialize();
    if(precice.isActionRequired(precice::constants::actionWriteInitialData())) {
        coupler.sendCouplingData();
        precice.fulfilledAction(precice::constants::actionWriteInitialData());
    }
    precice.initializeData();
    coupler.receiveCouplingData();

    Info<< "\nStarting time loop\n" << endl;

    while (simple.loop())
    {
        
        if(precice.isActionRequired(cowic)) {
            precice.fulfilledAction(cowic);
        }

        coupler.receiveCouplingData();

        // Pressure-velocity SIMPLE corrector
        {
            #include "UEqn.H"
            #include "EEqn.H"
            #include "pEqn.H"
        }

        turbulence->correct();

        coupler.sendCouplingData();
        
        std::cout << "Before advance" << std::endl;
        preciceDt = precice.advance(runTime.deltaT().value());
        std::cout << "After advance" << std::endl;
        
        runTime.write(); // write anyway!
        
        if(precice.isActionRequired(coric)) {
            
            precice.fulfilledAction(coric);
            
        } else {

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;
        }

    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
