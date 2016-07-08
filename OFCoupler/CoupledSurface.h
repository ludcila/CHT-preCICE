#ifndef COUPLEDSURFACE_H
#define COUPLEDSURFACE_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "CouplingDataUser/CouplingDataReader/CouplingDataReader.h"
#include "CouplingDataUser/CouplingDataWriter/CouplingDataWriter.h"
#include "precice/SolverInterface.hpp"


namespace ofcoupler
{

class CoupledSurface
{
protected:
    precice::SolverInterface & _precice;
    std::string _meshName;
    int _meshID;
    std::vector<std::string> _patchNames;
    std::vector<int> _patchIDs;
    int _numDataLocations;
    int * _vertexIDs;
    int _numDims;
    double * _dataBuffer;
    std::vector<CouplingDataReader*> _couplingDataReaders;
    std::vector<CouplingDataWriter*> _couplingDataWriters;
    void _configureMesh(fvMesh & mesh);
    void _configureBufferUser(CouplingDataUser & bufferUser);
public:
    CoupledSurface(precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::vector<std::string> patchNames);
    void receiveData();
    void sendData();
    void addCouplingDataWriter(std::string dataName, CouplingDataWriter * couplingDataWriter);
    void addCouplingDataReader(std::string dataName, CouplingDataReader * couplingDataReader);
};

}

#endif // COUPLEDSURFACE_H
