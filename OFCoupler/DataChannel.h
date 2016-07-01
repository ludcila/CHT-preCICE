#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <string>
namespace ofcoupler
{
class Interface;
}

namespace ofcoupler
{

class DataChannel
{

protected:
    int _dataID;
    std::string _dataName;
    Interface * _interface;
    bool _isVectorData = false;
public:
    DataChannel();
    void setInterface(Interface * interface);
    void setDataID(int id);
    void setDataName(std::string dataName);
    std::string dataName();
};

}

#endif // DATACHANNEL_H
