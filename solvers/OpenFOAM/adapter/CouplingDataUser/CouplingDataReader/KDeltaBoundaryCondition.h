#ifndef KDELTABOUNDARYCONDITION_H
#define KDELTABOUNDARYCONDITION_H

#include "fvCFD.H"
#include "CouplingDataReader.h"
#include "mixedFvPatchFields.H"
#include "turbulentFluidThermoModel.H"


namespace ofcoupler
{
class KDeltaBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
    autoPtr<compressible::turbulenceModel> & _turbulence;
public:
    KDeltaBoundaryCondition(volScalarField & T, autoPtr<compressible::turbulenceModel> & _turbulence);
    
    // CouplingDataReader interface
public:
    void read(double *dataBuffer);
};
}

#endif // KDELTABOUNDARYCONDITION_H
