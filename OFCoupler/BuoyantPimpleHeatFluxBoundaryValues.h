#ifndef BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H

#include "ScalarDataBufferWriter.h"
#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"

namespace ofcoupler
{

class BuoyantPimpleHeatFluxBoundaryValues : public ScalarDataBufferWriter
{

protected:
    volScalarField & _T;
    rhoThermo & _thermo;
    autoPtr<compressible::turbulenceModel> & _turbulence;
public:
    BuoyantPimpleHeatFluxBoundaryValues(volScalarField & T, rhoThermo & thermo, autoPtr<compressible::turbulenceModel> & turbulence);


    // DataBufferWriter interface
public:
    void writeToBuffer();
};

}

#endif // BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
