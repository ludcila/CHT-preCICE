#ifndef BUOYANTPIMPLEHEATFLUXBOUNDARYCONDITION_H
#define BUOYANTPIMPLEHEATFLUXBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "CouplingDataReader.h"
#include "fixedGradientFvPatchFields.H"

namespace adapter
{

class BuoyantPimpleHeatFluxBoundaryCondition : public CouplingDataReader
{

protected:

	volScalarField & _T;
	rhoThermo & _thermo;
	autoPtr<compressible::turbulenceModel> & _turbulence;

public:

	BuoyantPimpleHeatFluxBoundaryCondition( volScalarField & T,
											rhoThermo & thermo,
											autoPtr<compressible::turbulenceModel> & turbulence );

	void read( double * dataBuffer );

};

}


#endif // BUOYANTPIMPLEHEATFLUXBOUNDARYCONDITION_H
