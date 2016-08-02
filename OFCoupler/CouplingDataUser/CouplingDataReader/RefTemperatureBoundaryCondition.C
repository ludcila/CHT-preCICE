#include "RefTemperatureBoundaryCondition.h"

ofcoupler::RefTemperatureBoundaryCondition::RefTemperatureBoundaryCondition(volScalarField &T) :
    _T(T)
{
    
}

void ofcoupler::RefTemperatureBoundaryCondition::read(double * dataBuffer)
{
    
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        mixedFvPatchScalarField & TPatch = refCast<mixedFvPatchScalarField>(_T.boundaryField()[patchID]);
        forAll(TPatch, i) {
            TPatch.refValue()[i] = dataBuffer[bufferIndex++];
            std::cout << "read refValue(" << i << ") = " << TPatch.refValue()[i] << std::endl;
        }

    }
}
