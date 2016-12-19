#ifndef HEATTRANSFERCOEFFICIENTBOUNDARYCONDITION_H
#define HEATTRANSFERCOEFFICIENTBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "CouplingDataReader.h"
#include "mixedFvPatchFields.H"
#include "turbulentFluidThermoModel.H"


namespace adapter
{

template<typename autoPtrTurb>
class HeatTransferCoefficientBoundaryCondition : public CouplingDataReader
{

protected:

	volScalarField & _T;
	autoPtrTurb & _turbulence;

public:

	HeatTransferCoefficientBoundaryCondition( volScalarField & T, autoPtrTurb & _turbulence ) :
		_T( T ),
		_turbulence( _turbulence )
	{
		_dataType = scalar;
	}

public:

	void read( double * dataBuffer )
	{

		int bufferIndex = 0;

		for( uint k = 0 ; k < _patchIDs.size() ; k++ )
		{

			int patchID = _patchIDs.at( k );

			const fvPatch & kPatch =
				refCast<const fvPatch>( _turbulence->kappaEff() ().mesh().boundary()[patchID] );
            
			scalarField myKDelta = _turbulence->kappaEff() ().boundaryField()[patchID] * kPatch.deltaCoeffs();

			mixedFvPatchScalarField & TPatch = refCast<mixedFvPatchScalarField>( _T.boundaryField()[patchID] );
           
			forAll( TPatch, i )
			{
				double nbrKDelta = dataBuffer[bufferIndex++];
				TPatch.valueFraction()[i] = nbrKDelta / ( myKDelta[i] + nbrKDelta );
			}

		}
	}

};

}

#endif // HEATTRANSFERCOEFFICIENTBOUNDARYCONDITION_H
