#include "BuoyantPimpleHeatFluxBoundaryValues.h"


adapter::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues(volScalarField &T, rhoThermo &thermo, autoPtr<compressible::turbulenceModel> &turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}


void adapter::BuoyantPimpleHeatFluxBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;
    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField flux = - _turbulence->kappaEff()().boundaryField()[patchID]
                * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[patchID]).snGrad();
                
        forAll(flux, i) {
            dataBuffer[bufferIndex++] = flux[i];
        }

    }

}
