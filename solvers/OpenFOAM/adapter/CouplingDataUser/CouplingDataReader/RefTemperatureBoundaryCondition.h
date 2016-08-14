#ifndef REFTEMPERATUREBOUNDARYCONDITION_H
#define REFTEMPERATUREBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "CouplingDataReader.h"
#include "mixedFvPatchFields.H"


namespace ofcoupler
{
class RefTemperatureBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
public:
    RefTemperatureBoundaryCondition(volScalarField & T);
    
    // CouplingDataReader interface
public:
    void read(double *dataBuffer);
};
}

#endif // REFTEMPERATUREBOUNDARYCONDITION_H
