#ifndef BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H

#include "ScalarDataBufferWriter.h"
#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"

namespace ofcoupler
{

class BuoyantSimpleHeatFluxBoundaryValues : public ScalarDataBufferWriter
{

protected:
    volScalarField & _T;
    rhoThermo & _thermo;
    autoPtr<compressible::RASModel> & _turbulence;
public:
    BuoyantSimpleHeatFluxBoundaryValues(volScalarField & T, rhoThermo & thermo, autoPtr<compressible::RASModel> & turbulence);


    // DataBufferWriter interface
public:
    void writeToBuffer();
};

}

#endif // BUOYANTSIMPLEHEATFLUXBOUNDARYVALUES_H
