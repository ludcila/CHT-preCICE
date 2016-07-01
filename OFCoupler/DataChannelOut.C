#include "DataChannelOut.h"

ofcoupler::DataChannelOut::DataChannelOut(DataBufferWriter & bufferWriter) :
    _bufferWriter(bufferWriter)
{

}

ofcoupler::DataChannelOut::~DataChannelOut() {}
