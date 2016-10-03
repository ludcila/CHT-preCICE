#include "TemperatureBoundaryValues.h"

ofcoupler::TemperatureBoundaryValues::TemperatureBoundaryValues(volScalarField & T) :
    _T(T)
{

}

void ofcoupler::TemperatureBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;
    for(int k = 0; k < _patchIDs.size(); k++) {
        int patchID = _patchIDs.at(k);
        forAll(_T.boundaryField()[patchID], i) {
            dataBuffer[bufferIndex++] = _T.boundaryField()[patchID][i];
//            std::cout << dataBuffer[bufferIndex] << std::endl;
        }
    }
}
