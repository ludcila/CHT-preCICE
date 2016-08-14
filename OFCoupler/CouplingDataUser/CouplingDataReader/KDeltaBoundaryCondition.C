#include "KDeltaBoundaryCondition.h"

ofcoupler::KDeltaBoundaryCondition::KDeltaBoundaryCondition(volScalarField &T, autoPtr<compressible::turbulenceModel> &_turbulence) : 
    _T(T),
    _turbulence(_turbulence)
{
    
}

void ofcoupler::KDeltaBoundaryCondition::read(double *dataBuffer)
{
    
    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);
        
        const fvPatch & kPatch = refCast<const fvPatch>(_turbulence->kappaEff()().mesh().boundary()[patchID]);
        scalarField myKDelta = _turbulence->kappaEff()().boundaryField()[patchID] * kPatch.deltaCoeffs();
        
        mixedFvPatchScalarField & TPatch = refCast<mixedFvPatchScalarField>(_T.boundaryField()[patchID]);
        forAll(TPatch, i) {
            double nbrKDelta = dataBuffer[bufferIndex++];
            TPatch.valueFraction()[i] = nbrKDelta / (myKDelta[i] + nbrKDelta);
            std::cout << "read valueFraction(" << i << ") = " << TPatch.valueFraction()[i] << std::endl;
        }

    }
    
}
