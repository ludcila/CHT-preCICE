#ifndef SCALARDATACHANNELIN_H
#define SCALARDATACHANNELIN_H

#include "DataChannelIn.h"
#include "Interface.h"


namespace ofcoupler
{
class ScalarDataChannelIn : public DataChannelIn
{
public:
    ScalarDataChannelIn(DataBufferReader & bufferReader);

    // DataChannelIn interface
public:
    void receiveData();
};
}

#endif // SCALARDATACHANNELIN_H
