#include "SinkTemperatureBoundaryCondition.h"

adapter::SinkTemperatureBoundaryCondition::SinkTemperatureBoundaryCondition(volScalarField &T) :
    _T(T)
{
    
}

void adapter::SinkTemperatureBoundaryCondition::read(double * dataBuffer)
{
    
    int bufferIndex = 0;

    for(uint k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        mixedFvPatchScalarField & TPatch = refCast<mixedFvPatchScalarField>(_T.boundaryField()[patchID]);
        scalarField & rf = TPatch.refValue();
        forAll(TPatch, i) {
            rf[i] = dataBuffer[bufferIndex++];
//            std::cout << "read refValue(" << i << ") = " << TPatch.refValue()[i] << std::endl;
        }

    }
}
