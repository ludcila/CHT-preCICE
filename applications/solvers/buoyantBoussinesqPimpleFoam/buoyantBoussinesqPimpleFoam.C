/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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

#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "RASModel.H"
#include "radiationModel.H"
#include "fvIOoptionList.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"
#include "precice/SolverInterface.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "readGravitationalAcceleration.H"
    #include "createFields.H"
    #include "createIncompressibleRadiationModel.H"
    #include "createFvOptions.H"
    #include "initContinuityErrs.H"
    #include "readTimeControls.H"
    #include "CourantNo.H"
    #include "setInitialDeltaT.H"

    pimpleControl pimple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    /* =========================== Get mesh properties =========================== */
    
	label interfacePatchID = mesh.boundaryMesh().findPatchID("interface");
	const vectorField faceCenters = mesh.boundaryMesh()[interfacePatchID].faceCentres();
	int numVertices = faceCenters.size();
	
	/* Interface patch */
	fixedValueFvPatchScalarField & temperaturePatch = refCast<fixedValueFvPatchScalarField>(T.boundaryField()[interfacePatchID]);
   
    /* Interface data buffers */
    double temperatureBuffer[numVertices];
    double heatFluxBuffer[numVertices];
    scalarField temperatureField(numVertices);
    scalarField temperatureGradientField(numVertices);
    
    /* =========================== preCICE setup =========================== */
    
	precice::SolverInterface precice("Fluid", 0, 1);
	precice.configure("precice-config.xml");
	
	// Get preCICE IDs
	int meshID = precice.getMeshID("Fluid_Nodes");
	int temperatureID = precice.getDataID("Temperature", meshID);
	int heatFluxID = precice.getDataID("Heat_Flux", meshID);
	
	// Set mesh vertices
	double * vertices = new double[numVertices * 3];
	int vertexIDs[numVertices];
	forAll(faceCenters, i) {
		vertexIDs[i] = i;
		vertices[i*3 + 0] = faceCenters[i].x();
		vertices[i*3 + 1] = faceCenters[i].y();
		vertices[i*3 + 2] = faceCenters[i].z();
	}
	precice.setMeshVertices(meshID, numVertices, vertices, vertexIDs);
	
    /* =========================== preCICE initialize =========================== */
    
	double precice_dt = precice.initialize();
	precice.initializeData();
	
	const std::string& coric = precice::constants::actionReadIterationCheckpoint();
	const std::string& cowic = precice::constants::actionWriteIterationCheckpoint();

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "readTimeControls.H"
        #include "CourantNo.H"
        #include "setDeltaT.H"
        
        while(precice.isCouplingOngoing()) {
	    
			/* =========================== preCICE read data =========================== */
		
			if(precice.isActionRequired(cowic)){
				precice.fulfilledAction(cowic);
			}
			
			// Receive the temperature from the solid solver
			precice.readBlockScalarData(temperatureID, numVertices, vertexIDs, temperatureBuffer);
			
			// Set the temperature Dirichlet boundary condition
			forAll(temperatureField, i) {
				//std::cout << temperatureBuffer[i] << std::endl;
				temperatureField[i] = temperatureBuffer[i];
			}
			temperaturePatch == temperatureField;
			

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
		            turbulence->correct();
		        }
		    }
			
			/* =========================== preCICE write data =========================== */
		
		
			volScalarField alphaEff("alphaEff", turbulence->nu()/Pr + alphat);
			fvPatchScalarField aEff = alphaEff.boundaryField()[interfacePatchID];
			
			// Hard coded values
			double rho = 1;
			double Cp = 1;
			
			temperatureGradientField = temperaturePatch.snGrad();
			forAll(temperatureGradientField, i) {
				heatFluxBuffer[i] = aEff[i] * rho * Cp * temperatureGradientField[i];
			}
			precice.writeBlockScalarData(heatFluxID, numVertices, vertexIDs, heatFluxBuffer);
		
			precice_dt = precice.advance(precice_dt);
			
			if(precice.isActionRequired(coric)){
				precice.fulfilledAction(coric);
			}
		
			/* =========================== Done with preCICE =========================== */
		
            if ( precice.isTimestepComplete() ) {
                break;
            }
		}

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
