#include "DataChannel.h"

ofcoupler::DataChannel::DataChannel()
{

}

void ofcoupler::DataChannel::setInterface(ofcoupler::Interface *interface) { _interface = interface; }

void ofcoupler::DataChannel::setDataID(int id) { _dataID = id; }

void ofcoupler::DataChannel::setDataName(std::string dataName) { _dataName = dataName; }

std::string ofcoupler::DataChannel::dataName() { return _dataName; }
