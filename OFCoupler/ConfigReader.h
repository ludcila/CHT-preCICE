#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>
#include "yaml-cpp/yaml.h"
#include "Coupler.h"

namespace ofcoupler
{

class ConfigReader
{

    struct Data {
        std::string direction;
        std::string name;
    };

    struct Interface {
        std::string meshName;
        std::string patchName;
        std::vector<struct Data> data;
    };

protected:
    std::vector<struct Interface> _interfaces;
    std::string _preciceConfigFilename;

public:
    ConfigReader(std::string configFile, std::string participantName);
    std::vector<struct Interface> interfaces() { return _interfaces; }
    std::string preciceConfigFilename() { return _preciceConfigFilename; }
};

}

#endif // CONFIGREADER_H
