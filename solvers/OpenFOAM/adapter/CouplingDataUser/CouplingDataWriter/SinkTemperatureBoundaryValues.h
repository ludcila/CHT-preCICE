#ifndef SINKTEMPERATUREBOUNDARYVALUES_H
#define SINKTEMPERATUREBOUNDARYVALUES_H

#include "fvCFD.H"
#include "mixedFvPatchFields.H"
#include "CouplingDataWriter.h"


namespace adapter
{
class RefTemperatureBoundaryValues : public CouplingDataWriter
{
protected:

	volScalarField & _T;

public:

	RefTemperatureBoundaryValues( volScalarField & T );

	// CouplingDataWriter interface

public:

	void write( double * dataBuffer );
};

}

#endif // SINKTEMPERATUREBOUNDARYVALUES_H
