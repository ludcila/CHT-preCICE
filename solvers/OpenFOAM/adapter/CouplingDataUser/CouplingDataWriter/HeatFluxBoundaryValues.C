#include "HeatFluxBoundaryValues.h"


adapter::HeatFluxBoundaryValues::HeatFluxBoundaryValues( volScalarField & T, double k ) :
	_T( T ),
	_k( k )
{

}

void adapter::HeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		scalarField flux = -_k* refCast<fixedValueFvPatchScalarField>( _T.boundaryField()[patchID] ).snGrad();
		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}

