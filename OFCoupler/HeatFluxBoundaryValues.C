#include "HeatFluxBoundaryValues.h"


ofcoupler::HeatFluxBoundaryValues::HeatFluxBoundaryValues(volScalarField & T, double k) :
    _T(T),
    _k(k)
{

}

void ofcoupler::HeatFluxBoundaryValues::writeToBuffer()
{
    std::cout << "PATCH ID = "  << _patchID << std::endl;
    scalarField flux = _k * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[_patchID]).snGrad();
    forAll(flux, i) {
        _dataBuffer[i] = - flux[i];
    }
}


