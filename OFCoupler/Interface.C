#include "Interface.h"

ofcoupler::Interface::Interface(precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::string patchName) :
    _precice(precice),
    _meshName(meshName),
    _patchName(patchName),
    _numDims(3)
{
    _meshID = _precice.getMeshID(_meshName);
    _patchID = mesh.boundaryMesh().findPatchID(_patchName);
    _configureMesh(mesh);
    _dataBuffer = new double[_numDataLocations](); // TODO: check if it is vector data and allocate appropriately
}

void ofcoupler::Interface::_configureMesh(fvMesh & mesh) {
    const vectorField & faceCenters = mesh.boundaryMesh()[_patchID].faceCentres();
    _numDataLocations = faceCenters.size();
    double vertices[3 * _numDataLocations];
    for(int i = 0; i < _numDataLocations; i++) {
        vertices[3 * i + 0] = faceCenters[i].x();
        vertices[3 * i + 1] = faceCenters[i].y();
        vertices[3 * i + 2] = faceCenters[i].z();
    }
    _vertexIDs = new int[_numDataLocations];
    _precice.setMeshVertices(_meshID, _numDataLocations, vertices, _vertexIDs);
}

void ofcoupler::Interface::receiveData() {
    if(_precice.isReadDataAvailable()) {
        for(int i = 0; i < _dataChannelIns.size(); i++) {
            _dataChannelIns.at(i)->receiveData();
        }
    }
}

void ofcoupler::Interface::sendData() {
    for(int i = 0; i < _dataChannelOuts.size(); i++) {
        _dataChannelOuts.at(i)->sendData();
    }
}

void ofcoupler::Interface::_configureDataChannel(std::string dataName, DataChannel * dataChannel)
{
    dataChannel->setDataName(dataName);
    dataChannel->setInterface(this);
    dataChannel->setDataID(_precice.getDataID(dataName, _meshID));
}

void ofcoupler::Interface::_configureBufferUser(DataBufferUser & bufferUser)
{
    bufferUser.setBuffer(_dataBuffer);
    bufferUser.setSize(_numDataLocations);
    bufferUser.setPatchID(_patchID);
}

void ofcoupler::Interface::addDataChannel(std::string dataName, ofcoupler::ScalarDataBufferReader & bufferReader)
{
    DataChannelIn * dataChannelIn = new ofcoupler::ScalarDataChannelIn(bufferReader);
    _configureDataChannel(dataName, dataChannelIn);
    _configureBufferUser(bufferReader);
    _dataChannelIns.push_back(dataChannelIn);
}

void ofcoupler::Interface::addDataChannel(std::string dataName, ofcoupler::ScalarDataBufferWriter & bufferWriter)
{
    DataChannelOut * dataChannelOut = new ofcoupler::ScalarDataChannelOut(bufferWriter);
    _configureDataChannel(dataName, dataChannelOut);
    _configureBufferUser(bufferWriter);
    _dataChannelOuts.push_back(dataChannelOut);
}

precice::SolverInterface &ofcoupler::Interface::precice() { return _precice; }

int ofcoupler::Interface::numDataLocations() { return _numDataLocations; }

int *ofcoupler::Interface::vertexIDs() { return _vertexIDs; }

double *ofcoupler::Interface::dataBuffer() { return _dataBuffer; }

int ofcoupler::Interface::patchID() { return _patchID; }
