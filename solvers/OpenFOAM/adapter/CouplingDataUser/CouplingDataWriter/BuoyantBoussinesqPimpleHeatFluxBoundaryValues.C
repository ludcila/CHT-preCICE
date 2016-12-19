#include "BuoyantBoussinesqPimpleHeatFluxBoundaryValues.h"

adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryValues::BuoyantBoussinesqPimpleHeatFluxBoundaryValues( volScalarField & T, autoPtr<incompressible::RASModel> & turbulence, volScalarField & alphat, double Pr, double rho, double Cp ) :
	_T( T ),
	_turbulence( turbulence ),
	_alphat( alphat ),
	_Pr( Pr ),
	_rho( rho ),
	_Cp( Cp )
{
	_dataType = scalar;
}

void adapter::BuoyantBoussinesqPimpleHeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		scalarField alphaEff = _turbulence->nu() ().boundaryField()[patchID] /
							   _Pr + _alphat.boundaryField()[patchID];

		scalarField flux = -alphaEff* _rho* _Cp*
						   refCast<fixedValueFvPatchScalarField>( _T.boundaryField()[patchID] ).snGrad();

		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}

