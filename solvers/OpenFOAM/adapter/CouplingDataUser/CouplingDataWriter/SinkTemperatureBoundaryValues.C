#include "SinkTemperatureBoundaryValues.h"


adapter::SinkTemperatureBoundaryValues::SinkTemperatureBoundaryValues( volScalarField & T ) :
	_T( T )
{
    _dataType = scalar;
}

void adapter::SinkTemperatureBoundaryValues::write( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		fvPatchScalarField & TPatch = refCast<fvPatchScalarField>( _T.boundaryField()[patchID] );
		tmp<scalarField> patchInternalFieldTmp = TPatch.patchInternalField();
        scalarField & patchInternalField = patchInternalFieldTmp();

		forAll( TPatch, i )
		{
			dataBuffer[bufferIndex++] = patchInternalField[i];
		}

        patchInternalFieldTmp.clear();
        
	}
}

