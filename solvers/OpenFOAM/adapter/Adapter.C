#include "Adapter.h"

adapter::Adapter::Adapter(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName) :
    _precice(precice),
    _mesh(mesh),
    _solverName(solverName)
{

}

adapter::Interface &adapter::Adapter::addNewInterface(std::string meshName, std::vector<std::string> patchNames)
{
    adapter::Interface * interface = new adapter::Interface(_precice, _mesh, meshName, patchNames);
    _interface.push_back(interface);
    return *interface;
}

void adapter::Adapter::receiveCouplingData() {
    for(int i = 0; i < _interface.size(); i++) {
        _interface.at(i)->receiveData();
    }
}

void adapter::Adapter::sendCouplingData() {
    for(int i = 0; i < _interface.size(); i++) {
        _interface.at(i)->sendData();
    }
}
