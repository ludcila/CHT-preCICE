#ifndef SCALARDATABUFFERREADER_H
#define SCALARDATABUFFERREADER_H

#include "DataBufferReader.h"


namespace ofcoupler
{
class ScalarDataBufferReader : public DataBufferReader
{
protected:
    std::string _dataType = "scalar";
public:
    ScalarDataBufferReader();
};
}

#endif // SCALARDATABUFFERREADER_H
