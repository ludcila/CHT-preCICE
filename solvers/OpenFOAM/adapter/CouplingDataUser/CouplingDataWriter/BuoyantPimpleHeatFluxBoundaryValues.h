#ifndef BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "CouplingDataWriter.h"

namespace adapter
{

class BuoyantPimpleHeatFluxBoundaryValues : public CouplingDataWriter
{

protected:

	volScalarField & _T;
	rhoThermo & _thermo;
	autoPtr<compressible::turbulenceModel> & _turbulence;

public:

	BuoyantPimpleHeatFluxBoundaryValues( volScalarField & T,
										 rhoThermo & thermo,
										 autoPtr<compressible::turbulenceModel> & turbulence );
    
	void write( double * dataBuffer );

};

}

#endif // BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
