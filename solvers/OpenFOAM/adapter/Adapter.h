#ifndef COUPLER_H
#define COUPLER_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"
#include "Interface.h"

namespace adapter {

class Adapter
{

protected:
    precice::SolverInterface & _precice;
    std::string _solverName;
    std::vector<Interface*> _interface;
    fvMesh & _mesh;


public:
    Adapter(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName = "");
    precice::SolverInterface & precice() { return _precice; }
    Interface & addNewInterface(std::string meshName, std::vector<std::string> patchNames);
    void receiveCouplingData();
    void sendCouplingData();

};

}


#endif // COUPLER_H
