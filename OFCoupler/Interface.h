#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include "DataChannelIn.h"
#include "DataChannelOut.h"
#include "ScalarDataChannelIn.h"
#include "ScalarDataChannelOut.h"
#include "ScalarDataBufferReader.h"
#include "ScalarDataBufferWriter.h"
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"

//namespace ofcoupler
//{
//class DataChannel;
//class DataChannelIn;
//class DataChannelOut;
//class ScalarDataChannelIn;
//class ScalarDataChannelOut;
//}

namespace ofcoupler
{

class Interface
{
protected:
    precice::SolverInterface & _precice;
    std::string _meshName;
    int _meshID;
    std::string _patchName;
    int _patchID;
    int _numDataLocations;
    int * _vertexIDs;
    int _numDims;
    double * _dataBuffer;
    std::vector<DataChannelIn*> _dataChannelIns;
    std::vector<DataChannelOut*> _dataChannelOuts;
    void _configureMesh(fvMesh & mesh);
    void _configureDataChannel(std::string dataName, DataChannel * dataChannel);
    void _configureBufferUser(DataBufferUser & bufferUser);
public:
    Interface(precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::string patchName);
    void receiveData();
    void sendData();
    void addDataChannel(std::string dataName, ScalarDataBufferReader & bufferReader);
    void addDataChannel(std::string dataName, ScalarDataBufferWriter & bufferWriter);

    precice::SolverInterface & precice();
    int numDataLocations();
    int * vertexIDs();
    double * dataBuffer();
    int patchID();
};

}

#endif // INTERFACE_H
