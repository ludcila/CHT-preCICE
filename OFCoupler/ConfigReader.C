#include "ConfigReader.h"

ofcoupler::ConfigReader::ConfigReader(std::string configFile, std::string participantName)
{

    YAML::Node config = YAML::LoadFile(configFile);

    _preciceConfigFilename = config["precice-config-file"].as<std::string>();

    YAML::Node configInterfaces = config[participantName]["interfaces"];

    for(int i = 0; i < configInterfaces.size(); i++) {
        struct Interface interface;
        interface.meshName = configInterfaces[i]["mesh-name"].as<std::string>();
        interface.patchName = configInterfaces[i]["patch-name"].as<std::string>();
        for(int j = 0; j < configInterfaces[i]["data"].size(); j++) {
            struct Data data;
            data.name = configInterfaces[i]["data"][j]["name"].as<std::string>();
            data.direction = configInterfaces[i]["data"][j]["direction"].as<std::string>();
            interface.data.push_back(data);
        }
        _interfaces.push_back(interface);
    }

}
