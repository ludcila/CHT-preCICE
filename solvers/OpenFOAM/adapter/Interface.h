#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <boost/log/trivial.hpp>
#include "fvCFD.H"
#include "CouplingDataUser/CouplingDataReader/CouplingDataReader.h"
#include "CouplingDataUser/CouplingDataWriter/CouplingDataWriter.h"
#include "precice/SolverInterface.hpp"


namespace adapter
{

class Interface
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
	void _configureMesh( fvMesh & mesh );
	void _configureBufferUser( CouplingDataUser & bufferUser );

public:

	Interface( precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::vector<std::string> patchNames );
	void readCouplingData();
	void writeCouplingData();
	void addCouplingDataWriter( std::string dataName, CouplingDataWriter * couplingDataWriter );
	void addCouplingDataReader( std::string dataName, CouplingDataReader * couplingDataReader );
	~Interface();
};

}

#endif // INTERFACE_H
