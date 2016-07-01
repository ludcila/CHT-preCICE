#include "TemperatureBoundaryValues.h"

ofcoupler::TemperatureBoundaryValues::TemperatureBoundaryValues(volScalarField & T) :
    _T(T)
{

}

void ofcoupler::TemperatureBoundaryValues::writeToBuffer()
{
    forAll(_T.boundaryField()[_patchID], i) {
        _dataBuffer[i] = _T.boundaryField()[_patchID][i];
        std::cout << _dataBuffer[i] << std::endl;
    }
}
