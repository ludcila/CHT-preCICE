#ifndef TEMPERATUREBOUNDARYCONDITION_H
#define TEMPERATUREBOUNDARYCONDITION_H

#include "CouplingDataReader.h"
#include "fvCFD.H"

namespace adapter
{

class TemperatureBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
public:
    TemperatureBoundaryCondition(volScalarField & T);

    // CouplingDataReader interface
public:
    void read(double * dataBuffer);
};

}

#endif // TEMPERATUREBOUNDARYCONDITION_H
