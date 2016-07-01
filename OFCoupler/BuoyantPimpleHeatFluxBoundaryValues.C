#include "BuoyantPimpleHeatFluxBoundaryValues.h"


ofcoupler::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues(volScalarField &T, rhoThermo &thermo, autoPtr<compressible::turbulenceModel> &turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}

void ofcoupler::BuoyantPimpleHeatFluxBoundaryValues::writeToBuffer()
{

    scalarField flux = _turbulence->alphaEff()().boundaryField()[_patchID]
            * _thermo.Cp()().boundaryField()[_patchID]
            * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[_patchID]).snGrad();

    forAll(flux, i) {
//        double alphaEff = _turbulence->alphaEff()().boundaryField()[_patchID][i];
//        double Cp = _thermo.Cp()().boundaryField()[_patchID][i];
//        std::cout << alphaEff << " " << rho << " " << Cp << " " << alphaEff * Cp << alphaEff * rho * Cp << " " << alphaEff * rho * Cp * temperatureGradientField[i] << std::endl;
        _dataBuffer[i] = - flux[i];
    }
}
