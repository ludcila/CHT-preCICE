#ifndef HEATFLUXBOUNDARYCONDITION_H
#define HEATFLUXBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "fixedGradientFvPatchFields.H"
#include "CouplingDataReader.h"

namespace adapter
{
class HeatFluxBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
    double _k;
public:
    HeatFluxBoundaryCondition(volScalarField & T, double k);

    // CouplingDataReader interface
public:
    void read(double * dataBuffer);
};
}

#endif // HEATFLUXBOUNDARYCONDITION_H
