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
#include <sstream>
#include <vector>
#include <algorithm>
#include "yaml-cpp/yaml.h"
#include "adapter/ConfigReader.h"
#include "adapter/Adapter.h"
#include "adapter/Interface.h"
#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantPimpleHeatFluxBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"

#include "adapter/CouplingDataUser/CouplingDataReader/SinkTemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/SinkTemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/HeatTransferCoefficientBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/HeatTransferCoefficientBoundaryValues.h"



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    
    argList::addOption("precice-participant", "string", "name of preCICE participant");
    argList::addOption("precice-config", "string", "name of preCICE config file");
    argList::addBoolOption("disable-checkpointing", "disable checkpointing");

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
    bool checkpointingEnabled = ! args.optionFound("disable-checkpointing");
    adapter::ConfigReader config(preciceConfig, participantName);
    
    adapter::Adapter adapter(participantName, config.preciceConfigFilename(), mesh, runTime, "buoyantPimpleFoam");

    for(int i = 0; i < config.interfaces().size(); i++) {

        adapter::Interface & coupledSurface = adapter.addNewInterface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchNames);
        
        for(int j = 0; j < config.interfaces().at(i).writeData.size(); j++) {
            std::string dataName = config.interfaces().at(i).writeData.at(j);
            std::cout << dataName << std::endl;
            if(dataName.compare("Temperature") == 0) {
                adapter::TemperatureBoundaryValues * bw = new adapter::TemperatureBoundaryValues(thermo.T());
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.compare("Heat-Flux") == 0) {
                adapter::BuoyantPimpleHeatFluxBoundaryValues * bw = new adapter::BuoyantPimpleHeatFluxBoundaryValues(thermo.T(), thermo, turbulence);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.find("Heat-Transfer-Coefficient") == 0) {
                adapter::HeatTransferCoefficientBoundaryValues<autoPtr<compressible::turbulenceModel> > * bw = new adapter::HeatTransferCoefficientBoundaryValues<autoPtr<compressible::turbulenceModel> >(turbulence);
                coupledSurface.addCouplingDataWriter(dataName, bw);
            } else if(dataName.find("Sink-Temperature") == 0) {
                adapter::RefTemperatureBoundaryValues * bw = new adapter::RefTemperatureBoundaryValues(thermo.T());
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
                adapter::TemperatureBoundaryCondition * br = new adapter::TemperatureBoundaryCondition(thermo.T());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.compare("Heat-Flux") == 0) {
                adapter::BuoyantPimpleHeatFluxBoundaryCondition * br = new adapter::BuoyantPimpleHeatFluxBoundaryCondition(thermo.T(), thermo, turbulence);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.find("Heat-Transfer-Coefficient") == 0) {
                adapter::HeatTransferCoefficientBoundaryCondition<autoPtr<compressible::turbulenceModel> > * br = new adapter::HeatTransferCoefficientBoundaryCondition<autoPtr<compressible::turbulenceModel> >(thermo.T(), turbulence);
                coupledSurface.addCouplingDataReader(dataName, br);
            } else if(dataName.find("Sink-Temperature") == 0) {
                adapter::SinkTemperatureBoundaryCondition * br = new adapter::SinkTemperatureBoundaryCondition(thermo.T());
                coupledSurface.addCouplingDataReader(dataName, br);
            } else {
                std::cout << "Error: " << dataName << " does not exist." << std::endl;
                return 1;
            }
        }
        
    }

    // Chekpointing
    
    adapter.setCheckpointingEnabled(checkpointingEnabled);
    adapter.addCheckpointField(U);
    adapter.addCheckpointField(p);
    adapter.addCheckpointField(p_rgh);
    adapter.addCheckpointField(rho);
    adapter.addCheckpointField(thermo.T());
    adapter.addCheckpointField(thermo.he());
    //chkpt.addVolScalarField(thermo.hc()());
    adapter.addCheckpointField(thermo.p());
    adapter.addCheckpointField(K);
    adapter.addCheckpointField(dpdt);
    if(turbulenceUsed) {
        adapter.addCheckpointField(turbulence->k()());
        adapter.addCheckpointField(turbulence->epsilon()());
        adapter.addCheckpointField(turbulence->nut()());
        adapter.addCheckpointField(turbulence->alphat()());
        // chkpt.addVolScalarField(turbulence->mut()());
    }
    adapter.addCheckpointField(phi);

    adapter.initialize();
       
    Info<< "\nStarting time loop\n" << endl;

    while (adapter.isCouplingOngoing())
    {

        #include "createTimeControls.H"
        #include "compressibleCourantNo.H"
        #include "setDeltaT.H"
        
        adapter.adjustTimeStep();

        // Write checkpoint
        if(adapter.isWriteCheckpointRequired()){
            adapter.writeCheckpoint();
            adapter.fulfilledWriteCheckpoint();
        }

        runTime++;

        adapter.receiveCouplingData();

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
        
        adapter.sendCouplingData();
        
        adapter.advance();

        if(adapter.isReadCheckpointRequired()) {  
            
            adapter.readCheckpoint();
            if(turbulenceUsed && adapter.isCheckpointingEnabled()) {
                turbulence->alphat()().correctBoundaryConditions();
            }
            adapter.fulfilledReadCheckpoint();

        } else {

            runTime.write();

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

        }

        adapter.checkCouplingTimeStepComplete();
        
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
