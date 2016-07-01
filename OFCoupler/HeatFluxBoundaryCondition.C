#include "HeatFluxBoundaryCondition.h"

ofcoupler::HeatFluxBoundaryCondition::HeatFluxBoundaryCondition(volScalarField & T, double k) :
    _T(T),
    _k(k)
{
}

void ofcoupler::HeatFluxBoundaryCondition::readFromBuffer()
{
    fixedGradientFvPatchScalarField & gradientPatch = refCast<fixedGradientFvPatchScalarField>(_T.boundaryField()[_patchID]);
    forAll(gradientPatch, i) {
        gradientPatch.gradient()[i] = _dataBuffer[i] / _k;
    }
    Info << gradientPatch << endl;
}
