#ifndef SCALARDATABUFFERWRITER_H
#define SCALARDATABUFFERWRITER_H

#include "DataBufferWriter.h"


namespace ofcoupler
{
class ScalarDataBufferWriter : public DataBufferWriter
{
protected:
    std::string _dataType = "scalar";
public:
    ScalarDataBufferWriter();
};
}

#endif // SCALARDATABUFFERWRITER_H
