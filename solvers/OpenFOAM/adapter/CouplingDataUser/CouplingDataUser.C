#include "CouplingDataUser.h"

adapter::CouplingDataUser::CouplingDataUser()
{

}

std::string adapter::CouplingDataUser::direction() { return _direction; }

std::string adapter::CouplingDataUser::dataType() { return _dataType; }

bool adapter::CouplingDataUser::hasVectorData()
{
    return false; // TODO
}

bool adapter::CouplingDataUser::hasScalarData()
{
    return true; // TODO
}

void adapter::CouplingDataUser::setSize(int size)
{
    _bufferSize = size;
}

void adapter::CouplingDataUser::setPatchIDs(std::vector<int> patchIDs)
{
    _patchIDs = patchIDs;
}


void adapter::CouplingDataUser::setDataID(int dataID)
{
    _dataID = dataID;
}

int adapter::CouplingDataUser::dataID()
{
    return _dataID;
}
