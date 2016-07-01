#ifndef COUPLER_H
#define COUPLER_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"
#include "Interface.h"

namespace ofcoupler {

class Coupler
{

protected:
    precice::SolverInterface & _precice;
    std::string _solverName;
    std::vector<Interface*> _interfaces;
    fvMesh & _mesh;


public:
    Coupler(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName = "");
    precice::SolverInterface & precice() { return _precice; }
    Interface & addNewInterface(std::string meshName, std::string patchName);
    void receiveInterfaceData();
    void sendInterfaceData();

};

}


#endif // COUPLER_H
