#ifndef HEATFLUXBOUNDARYVALUES_H
#define HEATFLUXBOUNDARYVALUES_H

#include "ScalarDataBufferWriter.h"
#include "fvCFD.H"

namespace ofcoupler
{
class HeatFluxBoundaryValues : public ScalarDataBufferWriter
{
protected:
    volScalarField & _T;
    double _k;
public:
    HeatFluxBoundaryValues(volScalarField & T, double k);

    // DataBufferWriter interface
public:
    void writeToBuffer();
};
}

#endif // HEATFLUXBOUNDARYVALUES_H
