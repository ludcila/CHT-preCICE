#include "RefTemperatureBoundaryValues.h"


ofcoupler::RefTemperatureBoundaryValues::RefTemperatureBoundaryValues(volScalarField & T) :
    _T(T)
{
    
}

void ofcoupler::RefTemperatureBoundaryValues::write(double *dataBuffer)
{
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        fvPatchScalarField & TPatch = refCast<fvPatchScalarField>(_T.boundaryField()[patchID]);
        scalarField scf = TPatch.patchInternalField()();
        forAll(TPatch, i) {
            dataBuffer[bufferIndex++] = scf[i];
//            std::cout << "write refValue(" << i << ") = " << TPatch.patchInternalField()()[i] << std::endl;
        }

    }
}
