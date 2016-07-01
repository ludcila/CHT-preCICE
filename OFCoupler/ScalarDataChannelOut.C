#include "ScalarDataChannelOut.h"

ofcoupler::ScalarDataChannelOut::ScalarDataChannelOut(ofcoupler::DataBufferWriter & bufferWriter) :
    DataChannelOut(bufferWriter)
{

}

void ofcoupler::ScalarDataChannelOut::sendData()
{
    _bufferWriter.writeToBuffer();
    _interface->precice().writeBlockScalarData(_dataID, _interface->numDataLocations(), _interface->vertexIDs(), _interface->dataBuffer());
}
