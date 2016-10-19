#ifndef BUOYANTPIMPLESINKTEMPERATUREBOUNDARYVALUES_H
#define BUOYANTPIMPLESINKTEMPERATUREBOUNDARYVALUES_H


#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "CouplingDataWriter.h"


namespace adapter
{

class BuoyantPimpleSinkTemperatureBoundaryValues : public CouplingDataWriter
{
protected:

	volScalarField & _T;
	rhoThermo & _thermo;
	autoPtr<compressible::turbulenceModel> & _turbulence;

public:

	BuoyantPimpleSinkTemperatureBoundaryValues( volScalarField & T, rhoThermo & thermo, autoPtr<compressible::turbulenceModel> & turbulence );

	// CouplingDataWriter interface

public:

	void write( double * dataBuffer );
};

}

#endif // BUOYANTPIMPLESINKTEMPERATUREBOUNDARYVALUES_H
