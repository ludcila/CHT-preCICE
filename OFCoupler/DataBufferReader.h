#ifndef DATABUFFERREADER_H
#define DATABUFFERREADER_H

#include "DataBufferUser.h"


namespace ofcoupler
{
class DataBufferReader : public DataBufferUser
{
protected:
    std::string _direction = "in";
public:
    DataBufferReader();
    virtual void readFromBuffer() = 0;
    virtual ~DataBufferReader() {}
};
}

#endif // DATABUFFERREADER_H
