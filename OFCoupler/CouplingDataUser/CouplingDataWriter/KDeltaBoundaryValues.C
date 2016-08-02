#include "KDeltaBoundaryValues.h"

ofcoupler::KDeltaBoundaryValues::KDeltaBoundaryValues(autoPtr<compressible::turbulenceModel> & turbulence) :
    _turbulence(turbulence)
{
}

void ofcoupler::KDeltaBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;
    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        const fvPatch & kPatch = refCast<const fvPatch>(_turbulence->kappaEff()().mesh().boundary()[patchID]);
        
        scalarField kDelta = _turbulence->kappaEff()().boundaryField()[patchID] * kPatch.deltaCoeffs();

        std::cout << "KDelta" << std::endl;
        forAll(kDelta, i) {
            dataBuffer[bufferIndex++] = kDelta[i];
            std::cout << i << ") " << "write kDelta = " << kDelta[i] << std::endl;
        }

    }
}
