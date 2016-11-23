#ifndef SINKTEMPERATUREBOUNDARYCONDITION_H
#define SINKTEMPERATUREBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "CouplingDataReader.h"
#include "mixedFvPatchFields.H"


namespace adapter
{

class SinkTemperatureBoundaryCondition : public CouplingDataReader
{
    
protected:

	volScalarField & _T;

public:

	SinkTemperatureBoundaryCondition( volScalarField & T );
	void read( double * dataBuffer );
};

}

#endif // SINKTEMPERATUREBOUNDARYCONDITION_H
