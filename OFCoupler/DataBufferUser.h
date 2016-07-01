#ifndef DATABUFFERUSER_H
#define DATABUFFERUSER_H

#include <string>

namespace ofcoupler
{
class DataBufferUser
{
protected:
    std::string _direction;
    std::string _dataType;
    int _bufferSize; // if it is vector data, the real size in memory is (_bufferSize x dims)
    double * _dataBuffer;
    int _patchID;
public:
    DataBufferUser();
    std::string direction();
    std::string dataType();
    void setBuffer(double * buffer);
    void setSize(int size);
    void setPatchID(int patchID);

};
}

#endif // DATABUFFERUSER_H
