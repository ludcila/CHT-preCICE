#ifndef BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYCONDITION_H
#define BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYCONDITION_H

#include "CouplingDataReader.h"
#include "fvCFD.H"
#include "turbulentTransportModel.H"
#include "fixedGradientFvPatchFields.H"

namespace ofcoupler
{
class BuoyantBoussinesqPimpleHeatFluxBoundaryCondition : public CouplingDataReader
{
protected:
    volScalarField & _T;
    volScalarField & _alphat;
    double _Pr;
    double _rho;
    double _Cp;
    autoPtr<incompressible::RASModel> & _turbulence;
public:
    BuoyantBoussinesqPimpleHeatFluxBoundaryCondition(volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp);
    
    // CouplingDataReader interface
public:
    void read(double * dataBuffer);
};

}

#endif // BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYCONDITION_H
