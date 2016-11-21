#include "TemperatureBoundaryCondition.h"

adapter::TemperatureBoundaryCondition::TemperatureBoundaryCondition( volScalarField & T ) :
	_T( T )
{

}

void adapter::TemperatureBoundaryCondition::read( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		int patchID = _patchIDs.at( k );
		forAll( _T.boundaryField()[patchID], i )
		{
			_T.boundaryField()[patchID][i] = dataBuffer[bufferIndex++];
		}
	}
}

