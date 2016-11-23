#ifndef BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "turbulentTransportModel.H"
#include "CouplingDataWriter.h"


namespace adapter
{

class BuoyantBoussinesqPimpleHeatFluxBoundaryValues : public CouplingDataWriter
{

protected:

	volScalarField & _T;
	volScalarField & _alphat;
	double _Pr;
	double _rho;
	double _Cp;
	autoPtr<incompressible::RASModel> & _turbulence;

public:

	BuoyantBoussinesqPimpleHeatFluxBoundaryValues( volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp );
	void write( double * dataBuffer );

};

}

#endif // BUOYANTBOUSSINESQPIMPLEHEATFLUXBOUNDARYVALUES_H
