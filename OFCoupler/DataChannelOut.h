#ifndef DATACHANNELOUT_H
#define DATACHANNELOUT_H

#include "DataBufferWriter.h"
#include "DataChannel.h"


namespace ofcoupler
{
class DataChannelOut : public DataChannel
{
protected:
    DataBufferWriter & _bufferWriter;
public:
    DataChannelOut(DataBufferWriter & bufferWriter);
    virtual void sendData() = 0;
    virtual ~DataChannelOut();
};
}

#endif // DATACHANNELOUT_H
