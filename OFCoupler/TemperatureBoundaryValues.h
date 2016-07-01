#ifndef TEMPERATUREBOUNDARYVALUES_H
#define TEMPERATUREBOUNDARYVALUES_H

#include "fvCFD.H"
#include "ScalarDataBufferWriter.h"


namespace ofcoupler
{
class TemperatureBoundaryValues : public ScalarDataBufferWriter
{
protected:
    volScalarField & _T;
public:
    TemperatureBoundaryValues(volScalarField & T);

    // DataBufferWriter interface
public:
    void writeToBuffer();
};
}

#endif // TEMPERATUREBOUNDARYVALUES_H
