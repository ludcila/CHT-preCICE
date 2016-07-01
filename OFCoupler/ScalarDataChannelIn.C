#include "ScalarDataChannelIn.h"

ofcoupler::ScalarDataChannelIn::ScalarDataChannelIn(ofcoupler::DataBufferReader & bufferReader) :
    DataChannelIn(bufferReader)
{

}

void ofcoupler::ScalarDataChannelIn::receiveData()
{
    _interface->precice().readBlockScalarData(_dataID, _interface->numDataLocations(), _interface->vertexIDs(), _interface->dataBuffer());
    _bufferReader.readFromBuffer();
}
