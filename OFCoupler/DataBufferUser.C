#include "DataBufferUser.h"

ofcoupler::DataBufferUser::DataBufferUser()
{

}

std::string ofcoupler::DataBufferUser::direction() { return _direction; }

std::string ofcoupler::DataBufferUser::dataType() { return _dataType; }



void ofcoupler::DataBufferUser::setBuffer(double * buffer)
{
    _dataBuffer = buffer;
}

void ofcoupler::DataBufferUser::setSize(int size)
{
    _bufferSize = size;
}


void ofcoupler::DataBufferUser::setPatchID(int patchID)
{
    _patchID = patchID;
}
