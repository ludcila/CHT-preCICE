#ifndef TEMPERATUREBOUNDARYVALUES_H
#define TEMPERATUREBOUNDARYVALUES_H

#include "CouplingDataWriter.h"
#include "fvCFD.H"


namespace ofcoupler
{
class TemperatureBoundaryValues : public CouplingDataWriter
{
protected:
    volScalarField & _T;
public:
    TemperatureBoundaryValues(volScalarField & T);

    // CouplingDataWriter interface
public:
    void write(double * dataBuffer);
};
}

#endif // TEMPERATUREBOUNDARYVALUES_H
