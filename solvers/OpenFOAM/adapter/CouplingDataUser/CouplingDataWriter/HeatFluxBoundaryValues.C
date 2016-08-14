#include "HeatFluxBoundaryValues.h"


ofcoupler::HeatFluxBoundaryValues::HeatFluxBoundaryValues(volScalarField & T, double k) :
    _T(T),
    _k(k)
{

}

void ofcoupler::HeatFluxBoundaryValues::write(double * dataBuffer)
{

    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField flux = - _k * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[patchID]).snGrad();
        forAll(flux, i) {
            dataBuffer[bufferIndex++] = flux[i];
        }

    }

}
