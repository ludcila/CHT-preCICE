#ifndef SCALARDATACHANNELOUT_H
#define SCALARDATACHANNELOUT_H

#include "DataChannelOut.h"
#include "Interface.h"


namespace ofcoupler
{
class ScalarDataChannelOut : public DataChannelOut
{
public:
    ScalarDataChannelOut(DataBufferWriter & bufferWriter);

    // DataChannelOut interface
public:
    void sendData();
};
}

#endif // SCALARDATACHANNELOUT_H
