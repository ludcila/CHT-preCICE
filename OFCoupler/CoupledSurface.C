#include "CoupledSurface.h"

ofcoupler::CoupledSurface::CoupledSurface(precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::vector<std::string> patchNames) :
    _precice(precice),
    _meshName(meshName),
    _patchNames(patchNames),
    _numDataLocations(0),
    _numDims(3)
{
    _meshID = _precice.getMeshID(_meshName);
    for(int i = 0; i < patchNames.size(); i++) {
        _patchIDs.push_back(mesh.boundaryMesh().findPatchID(patchNames.at(i)));
    }
    _configureMesh(mesh);
    _dataBuffer = new double[_numDataLocations](); // TODO: check if it is vector data and allocate appropriately
}

void ofcoupler::CoupledSurface::_configureMesh(fvMesh & mesh) {

    for(int k = 0; k < _patchIDs.size(); k++) {
        _numDataLocations += mesh.boundaryMesh()[_patchIDs.at(k)].faceCentres().size();
    }

    int vertexIndex = 0;
    double vertices[3 * _numDataLocations];
    _vertexIDs = new int[_numDataLocations];
    for(int k = 0; k < _patchIDs.size(); k++) {
        const vectorField & faceCenters = mesh.boundaryMesh()[_patchIDs.at(k)].faceCentres();
        for(int i = 0; i < faceCenters.size(); i++) {
            vertices[vertexIndex++] = faceCenters[i].x();
            vertices[vertexIndex++] = faceCenters[i].y();
            vertices[vertexIndex++] = faceCenters[i].z();
        }
    }
    _precice.setMeshVertices(_meshID, _numDataLocations, vertices, _vertexIDs);

}

void ofcoupler::CoupledSurface::addCouplingDataWriter(std::string dataName, CouplingDataWriter * couplingDataWriter)
{
    couplingDataWriter->setDataID(_precice.getDataID(dataName, _meshID));
    couplingDataWriter->setPatchIDs(_patchIDs);
    _couplingDataWriters.push_back(couplingDataWriter);
}

void ofcoupler::CoupledSurface::addCouplingDataReader(std::string dataName, ofcoupler::CouplingDataReader * couplingDataReader)
{
    couplingDataReader->setDataID(_precice.getDataID(dataName, _meshID));
    couplingDataReader->setPatchIDs(_patchIDs);
    _couplingDataReaders.push_back(couplingDataReader);
}

void ofcoupler::CoupledSurface::receiveData() {
    if(_precice.isReadDataAvailable()) {
        for(int i = 0; i < _couplingDataReaders.size(); i++) {
            ofcoupler::CouplingDataReader * couplingDataReader = _couplingDataReaders.at(i);
            if(couplingDataReader->hasVectorData()) {
                _precice.readBlockVectorData(couplingDataReader->dataID(), _numDataLocations, _vertexIDs, _dataBuffer);
            } else {
                _precice.readBlockScalarData(couplingDataReader->dataID(), _numDataLocations, _vertexIDs, _dataBuffer);
            }
            couplingDataReader->read(_dataBuffer);
        }
    }
}

void ofcoupler::CoupledSurface::sendData() {
    for(int i = 0; i < _couplingDataWriters.size(); i++) {
        ofcoupler::CouplingDataWriter * couplingDataWriter = _couplingDataWriters.at(i);
        couplingDataWriter->write(_dataBuffer);
        if(couplingDataWriter->hasVectorData()) {
            _precice.writeBlockVectorData(couplingDataWriter->dataID(), _numDataLocations, _vertexIDs, _dataBuffer);
        } else {
            _precice.writeBlockScalarData(couplingDataWriter->dataID(), _numDataLocations, _vertexIDs, _dataBuffer);
        }
    }
}
