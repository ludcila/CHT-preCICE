#ifndef KDELTABOUNDARYCONDITION_H
#define KDELTABOUNDARYCONDITION_H

#include "fvCFD.H"
#include "CouplingDataReader.h"
#include "mixedFvPatchFields.H"
#include "turbulentFluidThermoModel.H"


namespace adapter
{
template<typename autoPtrTurb>
class KDeltaBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
    autoPtrTurb & _turbulence;
public:
    
    KDeltaBoundaryCondition(volScalarField &T, autoPtrTurb &_turbulence) : 
        _T(T),
        _turbulence(_turbulence)
    {
        
    }
    
    // CouplingDataReader interface
public:
    
    void read(double *dataBuffer)
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
//                std::cout << "read valueFraction(" << i << ") = " << TPatch.valueFraction()[i] << std::endl;
            }
    
        }
        
    }

    
}; // end class

} // end namespace

#endif // KDELTABOUNDARYCONDITION_H
