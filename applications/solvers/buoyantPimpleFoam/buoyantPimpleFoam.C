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
#include "precice/SolverInterface.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include "yaml-cpp/yaml.h"
#include "OFCoupler/ConfigReader.h"
#include "OFCoupler/Coupler.h"
#include "OFCoupler/Interface.h"
#include "OFCoupler/TemperatureBoundaryCondition.h"
#include "OFCoupler/BuoyantPimpleHeatFluxBoundaryValues.h"



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
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

    std::string participantName = runTime.caseName();

    ofcoupler::ConfigReader config("config.yml", participantName);

    precice::SolverInterface precice(participantName, 0, 1);
    precice.configure(config.preciceConfigFilename());
    ofcoupler::Coupler coupler(precice, mesh, "buoyantPimpleFoam");

    for(int i = 0; i < config.interfaces().size(); i++) {
        ofcoupler::Interface & interface = coupler.addNewInterface(config.interfaces().at(i).meshName, config.interfaces().at(i).patchName);
        for(int j = 0; j < config.interfaces().at(i).data.size(); j++) {
            std::string dataName = config.interfaces().at(i).data.at(j).name;
            std::string dataDirection = config.interfaces().at(i).data.at(j).direction;
            if(dataName.compare("Temperature") == 0) {
                if(dataDirection.compare("in") == 0) {
                    ofcoupler::TemperatureBoundaryCondition * br = new ofcoupler::TemperatureBoundaryCondition(thermo.T());
                    interface.addDataChannel(dataName, *br);
                } else {

                }
            } else if(dataName.compare("Heat-Flux") == 0) {
                if(dataDirection.compare("in") == 0) {

                } else {
                    ofcoupler::BuoyantPimpleHeatFluxBoundaryValues * bw = new ofcoupler::BuoyantPimpleHeatFluxBoundaryValues(thermo.T(), thermo, turbulence);
                    interface.addDataChannel(dataName, *bw);
                }
            }
        }
    }

    scalar couplingIterationTimeValue;
    label couplingIterationTimeIndex;

    // Chekpointing
    volVectorField U_checkpoint = U;
    volScalarField p_checkpoint = p;



    /* =========================== preCICE initialize =========================== */

    const std::string& coric = precice::constants::actionReadIterationCheckpoint();
    const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();


    double preciceDt = precice.initialize();
    precice.initializeData();
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

            if(solverDt.value() == preciceDt) {
                std::cout << "No subcycling" << std::endl;
            } else {
                std::cout << "Subcycling" << std::endl;
                U_checkpoint = U;
                p_checkpoint = p;
            }

            precice.fulfilledAction(cowic);
        }

        runTime++;

        Info<< "Time = " << runTime.timeName() << nl << endl;

        std::cout << U.oldTime().timeIndex() << std::endl;

        coupler.receiveInterfaceData();

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


        /* =========================== preCICE write data =========================== */


        coupler.sendInterfaceData();

        preciceDt = precice.advance(solverDt.value());

        if(precice.isActionRequired(coric)) {

            std::cout << ">>>>>> Read checkpoint required" << std::endl;
            bool noSubcycling = runTime.timeIndex() - couplingIterationTimeIndex == 1;

            // Set the time before copying the fields, in order to have the correct oldTime() field
            runTime.setTime(couplingIterationTimeValue, couplingIterationTimeIndex);

            if(noSubcycling) {
                std::cout << "No subcycling" << std::endl;
                // No need to manually reload the fields
            } else {
                std::cout << "Subcycling..." << std::endl;
                // Reload all fields
                U = U_checkpoint;
                p = p_checkpoint;
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
