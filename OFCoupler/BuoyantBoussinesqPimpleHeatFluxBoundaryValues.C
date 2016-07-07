#include "BuoyantBoussinesqPimpleHeatFluxBoundaryValues.h"

ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryValues::BuoyantBoussinesqPimpleHeatFluxBoundaryValues(volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp) :
    _T(T),
    _turbulence(turbulence),
    _alphat(alphat),
    _Pr(Pr),
    _rho(rho),
    _Cp(Cp)
{

}

void ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryValues::writeToBuffer()
{
    scalarField alphaEff = _turbulence->nu()().boundaryField()[_patchID] / _Pr + _alphat.boundaryField()[_patchID];
    scalarField flux = - alphaEff * _rho * _Cp * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[_patchID]).snGrad();
    Info << alphaEff *_rho*_Cp << endl;
    std::cout << "Flux" << std::endl;
    forAll(flux, i) {
        _dataBuffer[i] = flux[i];
        std::cout << flux[i] << std::endl;
    }
}

//volScalarField alphaEff("alphaEff", turbulence->nu()/Pr + alphat);
//fvPatchScalarField aEff = alphaEff.boundaryField()[interfacePatchID];

//// Hard coded values
//double rho = 1;
//double Cp = 1;
//double k = 1;

//if(precice.isWriteDataRequired(precice_dt)) {
//    temperatureGradientField = temperaturePatch.snGrad();
//    forAll(temperatureGradientField, i) {
//        //                double k = aEff[i] * rho * Cp;
//        heatFluxBuffer[i] = - k * temperatureGradientField[i];
//    }
//    precice.writeBlockScalarData(heatFluxID, numVertices, vertexIDs, heatFluxBuffer);
//    Info << temperatureGradientField << endl;
//}
