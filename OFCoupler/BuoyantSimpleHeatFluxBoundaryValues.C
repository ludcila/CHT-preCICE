#include "BuoyantSimpleHeatFluxBoundaryValues.h"


ofcoupler::BuoyantSimpleHeatFluxBoundaryValues::BuoyantSimpleHeatFluxBoundaryValues(volScalarField & T, rhoThermo & thermo, autoPtr<compressible::RASModel> & turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}

void ofcoupler::BuoyantSimpleHeatFluxBoundaryValues::writeToBuffer()
{
    scalarField flux = _turbulence->alphaEff()().boundaryField()[_patchID]
            * _thermo.Cp()().boundaryField()[_patchID]
            * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[_patchID]).snGrad();

    Info << flux << endl;

    forAll(flux, i) {
        _dataBuffer[i] = - flux[i];
    }
}
