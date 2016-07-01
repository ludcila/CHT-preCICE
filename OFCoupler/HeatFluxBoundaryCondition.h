#ifndef HEATFLUXBOUNDARYCONDITION_H
#define HEATFLUXBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "ScalarDataBufferReader.h"
#include "fixedGradientFvPatchFields.H"

namespace ofcoupler
{
class HeatFluxBoundaryCondition : public ScalarDataBufferReader
{
protected:
    volScalarField & _T;
    double _k;
public:
    HeatFluxBoundaryCondition(volScalarField & T, double k);

    // DataBufferReader interface
public:
    void readFromBuffer();
};
}

#endif // HEATFLUXBOUNDARYCONDITION_H
