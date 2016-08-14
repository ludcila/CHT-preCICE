#include "BuoyantPimpleHeatFluxBoundaryCondition.h"


ofcoupler::BuoyantPimpleHeatFluxBoundaryCondition::BuoyantPimpleHeatFluxBoundaryCondition(volScalarField &T, rhoThermo &thermo, autoPtr<compressible::turbulenceModel> &turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}

void ofcoupler::BuoyantPimpleHeatFluxBoundaryCondition::read(double *dataBuffer)
{
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField K = _turbulence->alphaEff()().boundaryField()[patchID] * _thermo.Cp()().boundaryField()[patchID];

        std::cout << "Flux" << std::endl;
        fixedGradientFvPatchScalarField & gradientPatch = refCast<fixedGradientFvPatchScalarField>(_T.boundaryField()[patchID]);
        forAll(gradientPatch, i) {
            std::cout << "q(" << i << ") = " << dataBuffer[bufferIndex] << "/" << K[i] << " = " << dataBuffer[bufferIndex] / K[i] << std::endl;
            gradientPatch.gradient()[i] = dataBuffer[bufferIndex++] / K[i];
        }

    }
}
