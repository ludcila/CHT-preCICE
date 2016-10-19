#ifndef BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "CouplingDataWriter.h"

namespace adapter
{

class BuoyantSimpleHeatFluxBoundaryValues : public CouplingDataWriter
{

protected:
    volScalarField & _T;
    rhoThermo & _thermo;
    autoPtr<compressible::RASModel> & _turbulence;
public:
    BuoyantSimpleHeatFluxBoundaryValues(volScalarField & T, rhoThermo & thermo, autoPtr<compressible::RASModel> & turbulence);



    // CouplingDataWriter interface
public:
    void write(double * dataBuffer);
}

#endif // BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H
