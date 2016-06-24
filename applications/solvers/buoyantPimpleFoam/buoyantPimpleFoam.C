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

class PreciceInterface {

protected:
    int _numVertices;
    int * _vertexIDs;
    double * _dataBuffer;
    int _meshID;
    int _readDataID;
    int _writeDataID;
    int _patchID;
    precice::SolverInterface * _precice;
    void _setMeshVertices(const vectorField & faceCenters);
public:
    PreciceInterface(precice::SolverInterface * precice, fvMesh & mesh, std::string interfaceName, std::string meshName, std::string readData, std::string writeData);
    virtual void readData() = 0;
    virtual void writeData(double solverDt) = 0;

};

PreciceInterface::PreciceInterface(precice::SolverInterface * precice, fvMesh & mesh, std::string interfaceName, std::string meshName, std::string readData, std::string writeData) {

    _precice = precice;
    _meshID = _precice->getMeshID(meshName);

    _patchID = mesh.boundaryMesh().findPatchID(interfaceName);
    _vertexIDs = new int[_numVertices];
    _setMeshVertices(mesh.boundaryMesh()[_patchID].faceCentres());
    _numVertices = mesh.boundaryMesh()[_patchID].faceCentres().size();

    _readDataID = _precice->getDataID(readData, _meshID);
    _writeDataID = _precice->getDataID(writeData, _meshID);

}

void PreciceInterface::_setMeshVertices(const vectorField & faceCenters) {
    double vertex[3];
    forAll(faceCenters, i) {
        vertex[0] = faceCenters[i].x();
        vertex[1] = faceCenters[i].y();
        vertex[2] = faceCenters[i].z();
        _precice->setMeshVertex(_meshID, vertex);
        _vertexIDs[i] = i;
    }
}

class TemperatureHeatFluxPreciceInterface : public PreciceInterface {

protected:
    rhoThermo * _thermo;
    autoPtr<compressible::turbulenceModel> & _turbulence;
    fixedValueFvPatchScalarField & _temperaturePatch;
public:
    TemperatureHeatFluxPreciceInterface(precice::SolverInterface * precice, fvMesh & mesh, rhoThermo * thermo, autoPtr<compressible::turbulenceModel> & turbulence, std::string interfaceName, std::string meshName);
    void readData();
    void writeData(double solverDt);

};

TemperatureHeatFluxPreciceInterface::TemperatureHeatFluxPreciceInterface(precice::SolverInterface * precice, fvMesh & mesh, rhoThermo * thermo, autoPtr<compressible::turbulenceModel> & turbulence, std::string interfaceName, std::string meshName)
    : PreciceInterface(precice, mesh, interfaceName, meshName, "Temperature", "Heat_Flux"),
        _thermo(thermo),
        _temperaturePatch(refCast<fixedValueFvPatchScalarField>(_thermo->T().boundaryField()[_patchID])),
        _turbulence(turbulence)
{
    _dataBuffer = new double[_numVertices];
}

void TemperatureHeatFluxPreciceInterface::readData() {

    scalarField temperatureField(_numVertices); // TODO: avoid this

    if(_precice->isReadDataAvailable()) {

        _precice->readBlockScalarData(_readDataID, _numVertices, _vertexIDs, _dataBuffer);

        forAll(temperatureField, i) {
            temperatureField[i] = _dataBuffer[i];
        }
        _temperaturePatch == temperatureField;
    }

}

void TemperatureHeatFluxPreciceInterface::writeData(double solverDt) {

    if(_precice->isWriteDataRequired(solverDt)) {
        scalarField temperatureGradientField = _temperaturePatch.snGrad();
        forAll(temperatureGradientField, i) {
            double alphaEff = _turbulence->alphaEff()().boundaryField()[_patchID][i];
            double rho = _thermo->rho().boundaryField()[_patchID][i];
            double Cp = _thermo->Cp()().boundaryField()[_patchID][i];
            _dataBuffer[i] = - alphaEff * rho * Cp * temperatureGradientField[i];
        }
        _precice->writeBlockScalarData(_writeDataID, _numVertices, _vertexIDs, _dataBuffer);
    }

}


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

    YAML::Node interfaces = YAML::LoadFile("interfaces-config.yml");
    
    std::string caseName = runTime.caseName();
    std::vector<std::string> interfacePatchNames;
    std::vector<std::string> interfaceMeshNames;

    int numInterfaces = interfaces[caseName].size();

    for(int i = 0; i < numInterfaces; i++) {
        interfacePatchNames.push_back(interfaces[caseName][i]["interface"].as<std::string>());
        interfaceMeshNames.push_back(interfaces[caseName][i]["faces-mesh"].as<std::string>());
    }

    /* =========================== preCICE setup =========================== */

    precice::SolverInterface precice(caseName, 0, 1);
    precice.configure("precice-config.xml");

    /* =========================== Setup the interfaces =========================== */

    std::vector<TemperatureHeatFluxPreciceInterface*> preciceInterfaces;
    
    for(int i = 0; i < numInterfaces; i++) {

        std::string interfacePatchName = interfacePatchNames.at(i);
        std::string interfaceMeshName = interfaceMeshNames.at(i);
        preciceInterfaces.push_back(new TemperatureHeatFluxPreciceInterface(&precice, mesh, &thermo, turbulence, interfacePatchName, interfaceMeshName));

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

        // Read and load interface data
        for(int i = 0; i < preciceInterfaces.size(); i++) {
            preciceInterfaces.at(i)->readData();
        }

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


        for(int i = 0; i < preciceInterfaces.size(); i++) {
            preciceInterfaces.at(i)->writeData(solverDt.value());
        }

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
