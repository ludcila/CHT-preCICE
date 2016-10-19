#include "BuoyantSimpleHeatFluxBoundaryValues.h"


adapter::BuoyantSimpleHeatFluxBoundaryValues::BuoyantSimpleHeatFluxBoundaryValues(volScalarField & T, rhoThermo & thermo, autoPtr<compressible::RASModel> & turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}

void adapter::BuoyantSimpleHeatFluxBoundaryValues::write(double * dataBuffer)
{

    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField flux = _turbulence->alphaEff()().boundaryField()[patchID]
                * _thermo.Cp()().boundaryField()[patchID]
                * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[patchID]).snGrad();

        Info << flux << endl;

        forAll(flux, i) {
            dataBuffer[bufferIndex++] = - flux[i];
        }

    }
}
