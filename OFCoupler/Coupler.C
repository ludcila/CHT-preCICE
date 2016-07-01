#include "Coupler.h"

ofcoupler::Coupler::Coupler(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName) :
    _precice(precice),
    _mesh(mesh),
    _solverName(solverName)
{

}

ofcoupler::Interface & ofcoupler::Coupler::addNewInterface(std::string meshName, std::string patchName) {
    ofcoupler::Interface * interface = new ofcoupler::Interface(_precice, _mesh, meshName, patchName);
    _interfaces.push_back(interface);
    return *interface;
}

void ofcoupler::Coupler::receiveInterfaceData() {
    for(int i = 0; i < _interfaces.size(); i++) {
        _interfaces.at(i)->receiveData();
    }
}

void ofcoupler::Coupler::sendInterfaceData() {
    for(int i = 0; i < _interfaces.size(); i++) {
        _interfaces.at(i)->sendData();
    }
}
