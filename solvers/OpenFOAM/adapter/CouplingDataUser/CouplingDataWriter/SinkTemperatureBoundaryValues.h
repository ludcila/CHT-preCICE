#ifndef SINKTEMPERATUREBOUNDARYVALUES_H
#define SINKTEMPERATUREBOUNDARYVALUES_H

#include "fvCFD.H"
#include "mixedFvPatchFields.H"
#include "CouplingDataWriter.h"


namespace adapter
{

class SinkTemperatureBoundaryValues : public CouplingDataWriter
{
    
protected:

	volScalarField & _T;

public:

	SinkTemperatureBoundaryValues( volScalarField & T );
	void write( double * dataBuffer );
    
};

}

#endif // SINKTEMPERATUREBOUNDARYVALUES_H
