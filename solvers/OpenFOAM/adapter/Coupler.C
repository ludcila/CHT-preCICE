#include "Coupler.h"

ofcoupler::Coupler::Coupler(precice::SolverInterface & precice, fvMesh & mesh, std::string solverName) :
    _precice(precice),
    _mesh(mesh),
    _solverName(solverName)
{

}

ofcoupler::CoupledSurface &ofcoupler::Coupler::addNewCoupledSurface(std::string meshName, std::vector<std::string> patchNames)
{
    ofcoupler::CoupledSurface * coupledSurface = new ofcoupler::CoupledSurface(_precice, _mesh, meshName, patchNames);
    _coupledSurfaces.push_back(coupledSurface);
    return *coupledSurface;
}

void ofcoupler::Coupler::receiveCouplingData() {
    for(int i = 0; i < _coupledSurfaces.size(); i++) {
        _coupledSurfaces.at(i)->receiveData();
    }
}

void ofcoupler::Coupler::sendCouplingData() {
    for(int i = 0; i < _coupledSurfaces.size(); i++) {
        _coupledSurfaces.at(i)->sendData();
    }
}
