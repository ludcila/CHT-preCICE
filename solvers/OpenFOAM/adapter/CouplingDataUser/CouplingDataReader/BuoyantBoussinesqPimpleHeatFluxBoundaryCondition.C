#include "BuoyantBoussinesqPimpleHeatFluxBoundaryCondition.h"

ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition(volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp) :
    _T(T),
    _turbulence(turbulence),
    _alphat(alphat),
    _Pr(Pr),
    _rho(rho),
    _Cp(Cp)
{

}

void ofcoupler::BuoyantBoussinesqPimpleHeatFluxBoundaryCondition::read(double * dataBuffer)
{

    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        scalarField alphaEff = _turbulence->nu()().boundaryField()[patchID] / _Pr + _alphat.boundaryField()[patchID];
        scalarField K = alphaEff * _rho * _Cp;

        fixedGradientFvPatchScalarField & gradientPatch = refCast<fixedGradientFvPatchScalarField>(_T.boundaryField()[patchID]);
        forAll(gradientPatch, i) {
            std::cout << dataBuffer[bufferIndex] << "/" << K[i] << " = " << dataBuffer[bufferIndex] / K[i] << std::endl;
            gradientPatch.gradient()[i] = dataBuffer[bufferIndex++] / K[i];
        }

    }
    
}
