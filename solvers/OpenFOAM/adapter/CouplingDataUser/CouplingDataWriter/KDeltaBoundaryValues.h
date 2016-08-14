#ifndef KDELTABOUNDARYVALUES_H
#define KDELTABOUNDARYVALUES_H

#include "fvCFD.H"
#include "CouplingDataWriter.h"
#include "turbulentFluidThermoModel.H"

namespace ofcoupler
{

class KDeltaBoundaryValues : public CouplingDataWriter
{
protected:
    autoPtr<compressible::turbulenceModel> & _turbulence;
    
public:
    KDeltaBoundaryValues(autoPtr<compressible::turbulenceModel> & turbulence);
    
    
    // CouplingDataWriter interface
public:
    void write(double *dataBuffer);
};

}

#endif // KDELTABOUNDARYVALUES_H
