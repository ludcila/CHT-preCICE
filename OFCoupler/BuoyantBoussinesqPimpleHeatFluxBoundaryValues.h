#ifndef BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "ScalarDataBufferWriter.h"
#include "turbulentTransportModel.H"


namespace ofcoupler
{
class BuoyantBoussinesqPimpleHeatFluxBoundaryValues : public ScalarDataBufferWriter
{
protected:
    volScalarField & _T;
    volScalarField & _alphat;
    double _Pr;
    double _rho;
    double _Cp;
    autoPtr<incompressible::RASModel> & _turbulence;
public:
    BuoyantBoussinesqPimpleHeatFluxBoundaryValues(volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp);


    // DataBufferWriter interface
public:
    void writeToBuffer();
};
}

#endif // BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H
