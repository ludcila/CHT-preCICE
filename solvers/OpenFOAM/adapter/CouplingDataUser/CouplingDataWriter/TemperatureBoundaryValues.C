#include "TemperatureBoundaryValues.h"

adapter::TemperatureBoundaryValues::TemperatureBoundaryValues( volScalarField & T ) :
	_T( T )
{

}

void adapter::TemperatureBoundaryValues::write( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		int patchID = _patchIDs.at( k );
		forAll( _T.boundaryField()[patchID], i )
		{
			dataBuffer[bufferIndex++] = _T.boundaryField()[patchID][i];
		}
	}
}

