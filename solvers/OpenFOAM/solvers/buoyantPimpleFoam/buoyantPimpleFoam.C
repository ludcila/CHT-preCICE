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

#include <mpi.h>
#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "radiationModel.H"
#include "fvIOoptionList.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"
#include "precice/SolverInterface.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include "yaml-cpp/yaml.h"
#include "adapter/ConfigReader.h"
#include "adapter/Coupler.h"
#include "adapter/CoupledSurface.h"
#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantPimpleHeatFluxBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantPimpleSinkTemperatureBoundaryValues.h"

#include "adapter/CouplingDataUser/CouplingDataReader/RefTemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/RefTemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/KDeltaBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/KDeltaBoundaryValues.h"



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    
    argList::addOption("precice-participant", "string", "name of preCICE participant");
    argList::addOption("precice-config", "string", "name of preCICE config file");

    #include "addRegionOption.H"

    #include "setRootCase.H"
    #include "createTime.H"
//    #include "createMesh.H"
    #include "createNamedMesh.H"

    pimpleControl pimple(mesh);

    #include "createFields.H"
    #include "createMRF.H"
    #include "createFvOptions.H"
    #include "createRadiationModel.H"
    #include "initContinuityErrs.H"
    #include "createTimeControls.H"
    #include "compressibleCourantNo.H"
    #include "setInitialDeltaT.H"
    
    bool turbulenceUsed = false;
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
        ).lookup("simulationType")
    );
    if(turbulenceModel != "laminar") turbulenceUsed = true;
    

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    std::string participantName = args.optionFound("precice-participant") ? args.optionRead<string>("precice-participant") : "Fluid";
    std::string preciceConfig = args.optionFound("precice-config") ? args.optionRead<string>("precice-config") : "config.yml";
    ofcoupler::ConfigReader config(preciceConfig, participantName);
    
    int mpiUsed, rank = 0, size = 1;
    MPI_Initialized(&mpiUsed);
    if(mpiUsed) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    precice::SolverInterface precice(participantName, rank, size);
    precice.configure(config.preciceConfigFilename());
    ofcoupler::Coupler coupler(precice, mesh, "buoyantPimpleFoam");

    for(int i = 0; i < config.interfaces().size(); i++) {

        ofcoupler::CoupledSurface & coupledSurface = coupler.addNewCoupledSurface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);
        
        for(int j = 0; j < config.interfaces().at(i).writeData.size(); j++) {
            std::string dataName = config.interfaces().at(i).writeData.at(j);
            std::cout << dataName << std::endl;
            if(dataName.compare("Temperature") == 0) {
                ofcoupler::TemperatureBoundaryValues * bw = new ofcoupler::TemperatureBoundaryValues(thermo.T());
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.compare("Heat-Flux") == 0) {
                ofcoupler::BuoyantPimpleHeatFluxBoundaryValues * bw = new ofcoupler::BuoyantPimpleHeatFluxBoundaryValues(thermo.T(), thermo, turbulence);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.find("kDelta-OF") == 0) {
                ofcoupler::KDeltaBoundaryValues<autoPtr<compressible::turbulenceModel> > * bw = new ofcoupler::KDeltaBoundaryValues<autoPtr<compressible::turbulenceModel> >(turbulence);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.find("kDelta-Temperature-OF") == 0) {
                ofcoupler::RefTemperatureBoundaryValues * bw = new ofcoupler::RefTemperatureBoundaryValues(thermo.T());
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
                ofcoupler::TemperatureBoundaryCondition * br = new ofcoupler::TemperatureBoundaryCondition(thermo.T());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.compare("Heat-Flux") == 0) {
                ofcoupler::BuoyantPimpleHeatFluxBoundaryCondition * br = new ofcoupler::BuoyantPimpleHeatFluxBoundaryCondition(thermo.T(), thermo, turbulence);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.find("kDelta-CCX") == 0) {
                ofcoupler::KDeltaBoundaryCondition<autoPtr<compressible::turbulenceModel> > * br = new ofcoupler::KDeltaBoundaryCondition<autoPtr<compressible::turbulenceModel> >(thermo.T(), turbulence);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.find("kDelta-Temperature-CCX") == 0) {
                ofcoupler::RefTemperatureBoundaryCondition * br = new ofcoupler::RefTemperatureBoundaryCondition(thermo.T());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else {
                std::cout << "Error: " << dataName << " does not exist." << std::endl;
                return 1;
            }
        }
        
    }
    
    scalar couplingIterationTimeValue;
    label couplingIterationTimeIndex;

    // Chekpointing
    volVectorField U_checkpoint = U;
    volScalarField p_checkpoint = p;
    volScalarField p_rgh_checkpoint = p_rgh;
    volScalarField rho_checkpoint = rho;
    volScalarField T_checkpoint = thermo.T();
    volScalarField he_checkpoint = thermo.he();
    volScalarField hc_checkpoint = thermo.hc()();
    volScalarField tp_checkpoint = thermo.p();
    volScalarField K_checkpoint = K;
    surfaceScalarField phi_checkpoint = phi;
    volScalarField dpdt_checkpoint = dpdt;
    volScalarField k_checkpoint = turbulence->k();
    volScalarField epsilon_checkpoint = turbulence->epsilon();
    volScalarField nut_checkpoint = turbulence->nut();
    volScalarField alphat_checkpoint = turbulence->alphat();
    volScalarField mut_checkpoint = turbulence->mut();

    /* =========================== preCICE initialize =========================== */

    const std::string& coric = precice::constants::actionReadIterationCheckpoint();
    const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();


    double preciceDt = precice.initialize();
    if(precice.isActionRequired(precice::constants::actionWriteInitialData())) {
        coupler.sendCouplingData();
        precice.fulfilledAction(precice::constants::actionWriteInitialData());
    }
    precice.initializeData();
    coupler.receiveCouplingData();
    
    dimensionedScalar solverDt("solverDt", dimensionSet(0,0,1,0,0,0,0), scalar(preciceDt));
    
    Info<< "\nStarting time loop\n" << endl;

    while (precice.isCouplingOngoing())
    {

        #include "createTimeControls.H"
        #include "compressibleCourantNo.H"
        #include "setDeltaT.H"

        // Set the solver timestep
        solverDt.value() = std::min(preciceDt, runTime.deltaT().value());
        runTime.setDeltaT(solverDt);

        // Write checkpoint
        if(precice.isActionRequired(cowic)){

            std::cout << "<<<<<< Write checkpoint required" << std::endl;

            couplingIterationTimeIndex = runTime.timeIndex();
            couplingIterationTimeValue = runTime.value();
            
            U_checkpoint = U;
            p_checkpoint = p;
            p_rgh_checkpoint = p_rgh;
            rho_checkpoint = rho;
            T_checkpoint = thermo.T();
            he_checkpoint = thermo.he();
            hc_checkpoint = thermo.hc()();
            tp_checkpoint = thermo.p();
            K_checkpoint = K;
            phi_checkpoint = phi;
            dpdt_checkpoint = dpdt;
            if(turbulenceUsed) {
                Info << "Writing turbulence checkpoing" << endl;
                k_checkpoint = turbulence->k()();
                epsilon_checkpoint = turbulence->epsilon()();
                nut_checkpoint = turbulence->nut()();
                mut_checkpoint = turbulence->mut()();
                turbulence->alphat()().correctBoundaryConditions();
                alphat_checkpoint = turbulence->alphat()();
            }
            

            if(solverDt.value() == preciceDt) {
                std::cout << "No subcycling" << std::endl;
            } else {
                std::cout << "Subcycling" << std::endl;
            }

            precice.fulfilledAction(cowic);
        }

        runTime++;

        std::cout << "Receive" << std::endl;
        coupler.receiveCouplingData();
        std::cout << "Finish receive" << std::endl;

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
        
        std::cout << "Send" << std::endl;
        coupler.sendCouplingData();
        std::cout << "Finish send" << std::endl;
        
        std::cout << "Advance" << std::endl;
        preciceDt = precice.advance(solverDt.value());
        std::cout << "Finish advance" << std::endl;

        if(precice.isActionRequired(coric)) {

            std::cout << ">>>>>> Read checkpoint required" << std::endl;
            bool noSubcycling = runTime.timeIndex() - couplingIterationTimeIndex == 1;

            // Set the time before copying the fields, in order to have the correct oldTime() field
            runTime.setTime(couplingIterationTimeValue, couplingIterationTimeIndex);
            
            U = U_checkpoint;
            p = p_checkpoint;
            rho = rho_checkpoint;
            p_rgh = p_rgh_checkpoint;
            thermo.T() = T_checkpoint;
            thermo.he() = he_checkpoint;
            thermo.hc()() = hc_checkpoint;
            thermo.p() = tp_checkpoint;
            K = K_checkpoint;
            phi = phi_checkpoint;
            dpdt = dpdt_checkpoint;
            if(turbulenceUsed) {
                Info << "Reading turbulence checkpoing" << endl;
                turbulence->k()() = k_checkpoint;
                turbulence->epsilon()() = epsilon_checkpoint;
                turbulence->nut()() = nut_checkpoint;
                turbulence->mut()() = mut_checkpoint;
                turbulence->alphat()() = alphat_checkpoint;
                turbulence->alphat()().correctBoundaryConditions();
            }
            
            if(noSubcycling) {
                std::cout << "No subcycling" << std::endl;
            } else {
                std::cout << "Subcycling..." << std::endl;
                // Reload all fields
            }

            std::cout << "Reset time = " << couplingIterationTimeValue << " (" << couplingIterationTimeIndex << ")" << std::endl;


            precice.fulfilledAction(coric);

        } else {

            runTime.write();

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

        }

        if(precice.isTimestepComplete()) {
            std::cout << "Coupling timestep completed!!!==================================================================================" << std::endl;
        }

    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
