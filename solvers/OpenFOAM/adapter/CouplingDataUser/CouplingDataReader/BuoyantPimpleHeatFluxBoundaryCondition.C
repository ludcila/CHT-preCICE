#include "BuoyantPimpleHeatFluxBoundaryCondition.h"
#include <boost/log/trivial.hpp>


adapter::BuoyantPimpleHeatFluxBoundaryCondition::BuoyantPimpleHeatFluxBoundaryCondition( volScalarField & T, rhoThermo & thermo, autoPtr<compressible::turbulenceModel> & turbulence ) :
	_T( T ),
	_thermo( thermo ),
	_turbulence( turbulence )
{

}

void adapter::BuoyantPimpleHeatFluxBoundaryCondition::read( double * dataBuffer )
{

	BOOST_LOG_TRIVIAL( info ) << "Setting heat flux boundary condition";

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		scalarField kappaEff = _turbulence->kappaEff() ().boundaryField()[patchID];

		fixedGradientFvPatchScalarField & gradientPatch = refCast<fixedGradientFvPatchScalarField>( _T.boundaryField()[patchID] );
		scalarField & gradient = gradientPatch.gradient();

		forAll( gradientPatch, i )
		{
			gradient[i] = dataBuffer[bufferIndex++] / kappaEff[i];
		}

	}
}

