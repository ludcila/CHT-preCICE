#include "CouplingDataUser.h"

ofcoupler::CouplingDataUser::CouplingDataUser()
{

}

std::string ofcoupler::CouplingDataUser::direction() { return _direction; }

std::string ofcoupler::CouplingDataUser::dataType() { return _dataType; }

bool ofcoupler::CouplingDataUser::hasVectorData()
{
    return false; // TODO
}

bool ofcoupler::CouplingDataUser::hasScalarData()
{
    return true; // TODO
}

void ofcoupler::CouplingDataUser::setSize(int size)
{
    _bufferSize = size;
}

void ofcoupler::CouplingDataUser::setPatchIDs(std::vector<int> patchIDs)
{
    _patchIDs = patchIDs;
}


void ofcoupler::CouplingDataUser::setDataID(int dataID)
{
    _dataID = dataID;
}

int ofcoupler::CouplingDataUser::dataID()
{
    return _dataID;
}
