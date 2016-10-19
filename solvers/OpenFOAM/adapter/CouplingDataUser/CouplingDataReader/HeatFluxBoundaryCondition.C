#include "HeatFluxBoundaryCondition.h"

adapter::HeatFluxBoundaryCondition::HeatFluxBoundaryCondition(volScalarField & T, double k) :
    _T(T),
    _k(k)
{
}

void adapter::HeatFluxBoundaryCondition::read(double * dataBuffer)
{
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        fixedGradientFvPatchScalarField & gradientPatch = refCast<fixedGradientFvPatchScalarField>(_T.boundaryField()[patchID]);
        forAll(gradientPatch, i) {
            gradientPatch.gradient()[i] = dataBuffer[bufferIndex++] / _k;
        }
        // Info << gradientPatch << endl;

    }

}
