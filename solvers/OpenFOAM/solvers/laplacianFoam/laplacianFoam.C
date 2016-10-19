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
    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    simpleControl simple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    int mpiUsed, rank = 0, size = 1;
    MPI_Initialized(&mpiUsed);
    if(mpiUsed) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }
    
    std::string participantName = runTime.caseName().components().first();
    adapter::ConfigReader config("config.yml", participantName);
    precice::SolverInterface precice(participantName, rank, size);
    precice.configure(config.preciceConfigFilename());
    adapter::Adapter adapter(precice, mesh, runTime, "laplacianFoam");

    for(int i = 0; i < config.interfaces().size(); i++) {

        adapter::Interface & coupledSurface = adapter.addNewInterface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);
        for(int j = 0; j < config.interfaces().at(i).data.size(); j++) {
            std::string dataName = config.interfaces().at(i).data.at(j).name;
            std::string dataDirection = config.interfaces().at(i).data.at(j).direction;
            if(dataName.compare("Temperature") == 0 && dataDirection.compare("out") == 0) {
                adapter::TemperatureBoundaryValues * bw = new adapter::TemperatureBoundaryValues(T);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.compare("Temperature") == 0 && dataDirection.compare("in") == 0) {
                adapter::TemperatureBoundaryCondition * br = new adapter::TemperatureBoundaryCondition(T);
                coupledSurface.addCouplingDataReader(dataName, br);
            }
            if(dataName.compare("Heat-Flux") == 0 && dataDirection.compare("in") == 0) {
                adapter::HeatFluxBoundaryCondition * br = new adapter::HeatFluxBoundaryCondition(T, k.value());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.compare("Heat-Flux") == 0 && dataDirection.compare("out") == 0) {
                adapter::HeatFluxBoundaryValues * bw = new adapter::HeatFluxBoundaryValues(T, k.value());
                coupledSurface.addCouplingDataWriter(dataName, bw);
            }
        }
    }
    
    double precice_dt = precice.initialize();
    precice.initializeData();
	
	const std::string& coric = precice::constants::actionReadIterationCheckpoint();
	const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();
	
    simple.loop();

    Info<< "\nCalculating temperature distribution\n" << endl;
	    
    while(precice.isCouplingOngoing()) {

        /* =========================== preCICE read data =========================== */

        if(precice.isActionRequired(cowic)){
            precice.fulfilledAction(cowic);
        }

        adapter.receiveCouplingData();

        /* =========================== solve =========================== */

        while (simple.correctNonOrthogonal())
        {
            solve
            (
                fvm::ddt(T) - fvm::laplacian(k/rho/Cp, T)
            );
        }

        /* =========================== preCICE write data =========================== */

        adapter.sendCouplingData();

        precice_dt = precice.advance(precice_dt);

        if(precice.isActionRequired(coric)){

            precice.fulfilledAction(coric);

        } else {

            #include "write.H"

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

            // Advance in time
            simple.loop();

        }
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
