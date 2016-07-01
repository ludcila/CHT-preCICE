#ifndef DATACHANNELIN_H
#define DATACHANNELIN_H

#include "DataBufferReader.h"
#include "DataChannel.h"

namespace ofcoupler
{
class DataChannelIn : public DataChannel
{
protected:
    DataBufferReader & _bufferReader;
public:
    DataChannelIn(DataBufferReader & bufferReader);
    virtual void receiveData() = 0;
    virtual ~DataChannelIn();
};
}

#endif // DATACHANNELIN_H
