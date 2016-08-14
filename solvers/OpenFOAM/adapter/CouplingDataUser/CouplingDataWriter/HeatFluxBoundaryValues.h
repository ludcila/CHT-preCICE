#ifndef HEATFLUXBOUNDARYVALUES_H
#define HEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "CouplingDataWriter.h"

namespace ofcoupler
{
class HeatFluxBoundaryValues : public CouplingDataWriter
{
protected:
    volScalarField & _T;
    double _k;
public:
    HeatFluxBoundaryValues(volScalarField & T, double k);


    // CouplingDataWriter interface
public:
    void write(double * dataBuffer);
};
}

#endif // HEATFLUXBOUNDARYVALUES_H
