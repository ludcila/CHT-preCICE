#ifndef COUPLER_H
#define COUPLER_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"
#include "CoupledSurface.h"

namespace ofcoupler {

class Coupler
{

protected:
    precice::SolverInterface & _precice;
    std::string _solverName;
    std::vector<CoupledSurface*> _coupledSurfaces;
    fvMesh & _mesh;


public:
    Coupler(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName = "");
    precice::SolverInterface & precice() { return _precice; }
    CoupledSurface & addNewCoupledSurface(std::string meshName, std::vector<std::string> patchNames);
    void receiveCouplingData();
    void sendCouplingData();

};

}


#endif // COUPLER_H
