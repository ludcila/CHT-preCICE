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

#include "fvCFD.H"
#include "simpleControl.H"
#include "precice/SolverInterface.hpp"
#include "fixedGradientFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    simpleControl simple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    /* =========================== Get mesh properties =========================== */
    
	label interfacePatchID = mesh.boundaryMesh().findPatchID("interface");
	const vectorField faceCenters = mesh.boundaryMesh()[interfacePatchID].faceCentres();
	int numVertices = faceCenters.size();
	
	/* Interface patch */
	fixedGradientFvPatchScalarField & temperatureGradientPatch = refCast<fixedGradientFvPatchScalarField>(T.boundaryField()[interfacePatchID]);
  
    /* Interface data buffers */
    double temperatureBuffer[numVertices];
    double heatFluxBuffer[numVertices];
    scalarField temperatureField(numVertices);
    scalarField temperatureGradientField(numVertices);
    
    /* =========================== preCICE setup =========================== */
    
    std::string caseName = runTime.caseName();
    precice::SolverInterface precice(caseName, 0, 1);
	precice.configure("precice-config.xml");
	
	// Get preCICE IDs
	int meshID = precice.getMeshID("Solid_Nodes");
	int temperatureID = precice.getDataID("Temperature", meshID);
    int heatFluxID = precice.getDataID("Heat-Flux", meshID);
	
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
	

    Info<< "\nCalculating temperature distribution\n" << endl;

    while (simple.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;
	    
        while(precice.isCouplingOngoing()) {
		    
			/* =========================== preCICE read data =========================== */

			if(precice.isActionRequired(cowic)){
				precice.fulfilledAction(cowic);
			}
		
            // Receive the heat flux from the fluid solver
            if(precice.isReadDataAvailable()) {
                precice.readBlockScalarData(heatFluxID, numVertices, vertexIDs, heatFluxBuffer);
                // Compute gradient from heat flux
                forAll(temperatureGradientPatch, i) {
                    temperatureGradientPatch.gradient()[i] = heatFluxBuffer[i] / k;
                }
                //Info << temperatureGradientPatch.gradient() << endl;
            }
			
			// Info << temperatureGradientPatch.gradient() << endl;

			/* =========================== solve =========================== */

		    while (simple.correctNonOrthogonal())
		    {
		        solve
		        (
                    fvm::ddt(T) - fvm::laplacian(k/rho/Cp, T)
		        );
		    }
		
			/* =========================== preCICE write data =========================== */
		
			forAll(T.boundaryField()[interfacePatchID], i) {
				temperatureBuffer[i] = T.boundaryField()[interfacePatchID][i];
                std::cout << temperatureBuffer[i] << std::endl;
			}
            precice.writeBlockScalarData(temperatureID, numVertices, vertexIDs, temperatureBuffer);
		
			precice_dt = precice.advance(precice_dt);
			
			if(precice.isActionRequired(coric)){
				precice.fulfilledAction(coric);
			}
		
			/* =========================== Done with preCICE =========================== */
		
            if ( precice.isTimestepComplete() )
                break;
		}

        #include "write.H"

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
