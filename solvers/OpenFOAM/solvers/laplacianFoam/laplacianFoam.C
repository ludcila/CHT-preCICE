/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
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
    laplacianFoam

Description
    Solves a simple Laplace equation, e.g. for thermal diffusion in a solid.

\*---------------------------------------------------------------------------*/

#include "mpi/mpi.h"
#include "fvCFD.H"
#include "simpleControl.H"
#include "precice/SolverInterface.hpp"
#include "fixedGradientFvPatchFields.H"
#include "adapter/ConfigReader.h"
#include "adapter/Adapter.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/HeatFluxBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/HeatFluxBoundaryCondition.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addOption("precice-participant", "string", "name of preCICE participant");
    argList::addOption("precice-config", "string", "name of preCICE config file");
    
    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    simpleControl simple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    
    std::string participantName = args.optionFound("precice-participant") ? args.optionRead<string>("precice-participant") : "Fluid";
    std::string preciceConfig = args.optionFound("precice-config") ? args.optionRead<string>("precice-config") : "config.yml";
    
    adapter::ConfigReader config(preciceConfig, participantName);
   
    bool subcyclingEnabled = true;
    adapter::Adapter adapter(participantName, config.preciceConfigFilename(), mesh, runTime, "laplacianFoam", subcyclingEnabled);

    for(int i = 0; i < config.interfaces().size(); i++) {

        adapter::Interface & coupledSurface = adapter.addNewInterface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);
        
        
        for(int j = 0; j < config.interfaces().at(i).writeData.size(); j++) {
            std::string dataName = config.interfaces().at(i).writeData.at(j);
            std::cout << dataName << std::endl;
            if(dataName.compare("Temperature") == 0) {
                adapter::TemperatureBoundaryValues * bw = new adapter::TemperatureBoundaryValues(T);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.compare("Heat-Flux") == 0) {
                adapter::HeatFluxBoundaryValues * bw = new adapter::HeatFluxBoundaryValues(T, k.value());
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
                adapter::TemperatureBoundaryCondition * br = new adapter::TemperatureBoundaryCondition(T);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.compare("Heat-Flux") == 0) {
                adapter::HeatFluxBoundaryCondition * br = new adapter::HeatFluxBoundaryCondition(T, k.value());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else {
                std::cout << "Error: " << dataName << " does not exist." << std::endl;
                return 1;
            }
        }
        
    }
    
    adapter.addCheckpointField(T);
    adapter.initialize();

    Info<< "\nCalculating temperature distribution\n" << endl;
	    
    while(adapter.isCouplingOngoing()) {
        
        adapter.adjustSolverTimeStep();

        if(adapter.isWriteCheckpointRequired()){
            adapter.writeCheckpoint();
            adapter.fulfilledWriteCheckpoint();
        }
        
        simple.loop();

        adapter.readCouplingData();

        /* =========================== solve =========================== */

        while (simple.correctNonOrthogonal())
        {
            solve
            (
                fvm::ddt(T) - fvm::laplacian(k/rho/Cp, T)
            );
        }

        /* =========================== preCICE write data =========================== */

        adapter.writeCouplingData();
        adapter.advance();

        if(adapter.isReadCheckpointRequired()){
            
            adapter.readCheckpoint();
            adapter.fulfilledReadCheckpoint();
            
        } else {
            
            #include "write.H"

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

        }
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
