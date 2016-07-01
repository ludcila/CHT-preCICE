#ifndef OFCOUPLER_H
#define OFCOUPLER_H

//#include "fvCFD.H"
//#include <vector>
//#include <string>
//#include "precice/SolverInterface.hpp"

//namespace ofcoupler {


//class Interface;


//class DataChannel
//{

//protected:
//    int _dataID;
//    Interface & _interface;
//    precice::SolverInterface * _precice;
//    bool _dataIsVector;

//public:
//    DataChannel(Interface & interface, std::string dataName);
//    virtual void update() = 0;
//    bool dataIsVector() { return _dataIsVector; }
//    virtual ~DataChannel();

//};

//class DataChannelIn : public DataChannel {

//protected:
//    virtual void _read() = 0;
//    virtual void _preciceRead(double * dataBuffer) = 0;
//public:
//    void update() { _read(); }
//    virtual ~DataChannelIn();

//};

//class ScalarDataChannelIn : public DataChannelIn {
//protected:
//    virtual void _preciceRead(double * dataBuffer);
//public:
//    bool dataIsVector() { return false; }
//};

//class VectorDataChannelIn : public DataChannelIn {
//protected:
//    virtual void _preciceRead(double * dataBuffer);
//public:
//    bool dataIsVector() { return true; }
//};

//class DataChannelOut : public DataChannel {

//protected:
//    virtual void _write() = 0;
//public:
//    void update() { _write(); }
//    virtual ~DataChannelOut();

//};

//class ScalarDataChannelOut : public DataChannelOut {
//public:
//    bool dataIsVector() { return false; }
//};

//class VectorDataChannelOut : public DataChannelOut {
//public:
//    bool dataIsVector() { return true; }
//};

//class Interface {

//protected:
//    std::vector<DataChannel*> _dataChannels;
//    int _size;
//    int * _valueIndices;
//    int _meshID;
//    double * _dataBuffer;
//    void _setMeshVertices();
//public:
//    int size() { return _size; }
//    int * valueIndices() { return _valueIndices; }
//    double * dataBuffer() { return _dataBuffer; }

//};

//class Coupler
//{

//protected:
//    precice::SolverInterface * _precice;
//    fvMesh * _ofMesh;

//public:
//    Coupler();
//    void read();
//    void write();

//};

//class BuoyantPimpleHeatFluxDataChannelOut : public DataChannelOut {
//protected:
//    virtual void _write();
//};

//class BuoyantPimpleFoamTemperatureDataChannelIn : public ScalarDataChannelIn {
//public:
//    BuoyantPimpleFoamTemperatureDataChannelIn(std::string dataName, volScalarField &field);
//protected:
//    volScalarField _tempField;
////    fixedValueFvPatchField _patchField;
//    virtual void _read();
//};


//}


//ofcoupler::DataChannel::DataChannel(ofcoupler::Interface & interface, std::string dataName) :
//    _interface(interface)
//{
//}

//void ofcoupler::ScalarDataChannelIn::_preciceRead(double * dataBuffer) {
//        _precice->readBlockScalarData(_dataID, _interface.size(), _interface.valueIndices(), dataBuffer);
//}

//void ofcoupler::VectorDataChannelIn::_preciceRead(double * dataBuffer) {
//        _precice->readBlockVectorData(_dataID, _interface.size(), _interface.valueIndices(), dataBuffer);
//}

//ofcoupler::BuoyantPimpleFoamTemperatureDataChannelIn::BuoyantPimpleFoamTemperatureDataChannelIn(std::string dataName, volScalarField &field) :
//    DataChannel(dataName) {

//}

//void ofcoupler::BuoyantPimpleFoamTemperatureDataChannelIn::_read() {
//    _preciceRead(_interface.dataBuffer());
//    _tempField.setSize(_interface.size());
////    forAll(_patchField, i) {
////        _tempField[i] = _interface.dataBuffer()[i];
////    }
////    _patchField == _tempField;
//}

#endif // OFCOUPLER_H
