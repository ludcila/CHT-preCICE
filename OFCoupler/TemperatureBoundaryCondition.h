#ifndef TEMPERATUREBOUNDARYCONDITION_H
#define TEMPERATUREBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "ScalarDataBufferReader.h"

namespace ofcoupler
{

class TemperatureBoundaryCondition : public ScalarDataBufferReader
{
protected:
    volScalarField & _T;
public:
    TemperatureBoundaryCondition(volScalarField & T);

    // DataBufferReader interface
public:
    void readFromBuffer();
};

}

#endif // TEMPERATUREBOUNDARYCONDITION_H
