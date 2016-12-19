#include "CouplingDataUser.h"
#include <iostream>

adapter::CouplingDataUser::CouplingDataUser()
{

}

bool adapter::CouplingDataUser::hasVectorData()
{
	return _dataType == vector;
}

bool adapter::CouplingDataUser::hasScalarData()
{
	return _dataType == scalar;
}

void adapter::CouplingDataUser::setSize( int size )
{
	_bufferSize = size;
}

void adapter::CouplingDataUser::setPatchIDs( std::vector<int> patchIDs )
{
	_patchIDs = patchIDs;
}

void adapter::CouplingDataUser::setDataID( int dataID )
{
	_dataID = dataID;
}

int adapter::CouplingDataUser::dataID()
{
	return _dataID;
}

