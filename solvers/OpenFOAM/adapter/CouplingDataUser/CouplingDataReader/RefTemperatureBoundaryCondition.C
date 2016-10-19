#include "RefTemperatureBoundaryCondition.h"

adapter::RefTemperatureBoundaryCondition::RefTemperatureBoundaryCondition(volScalarField &T) :
    _T(T)
{
    
}

void adapter::RefTemperatureBoundaryCondition::read(double * dataBuffer)
{
    
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        mixedFvPatchScalarField & TPatch = refCast<mixedFvPatchScalarField>(_T.boundaryField()[patchID]);
        scalarField & rf = TPatch.refValue();
        forAll(TPatch, i) {
            rf[i] = dataBuffer[bufferIndex++];
//            std::cout << "read refValue(" << i << ") = " << TPatch.refValue()[i] << std::endl;
        }

    }
}
