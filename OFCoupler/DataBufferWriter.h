#ifndef DATABUFFERWRITER_H
#define DATABUFFERWRITER_H

#include "DataBufferUser.h"


namespace ofcoupler
{
class DataBufferWriter : public DataBufferUser
{
protected:
    std::string _direction = "out";
public:
    DataBufferWriter();
    virtual void writeToBuffer() = 0;
    virtual ~DataBufferWriter() {}
};
}

#endif // DATABUFFERWRITER_H
