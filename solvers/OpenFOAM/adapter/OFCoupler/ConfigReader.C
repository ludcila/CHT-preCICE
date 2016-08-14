#include "ConfigReader.h"

ofcoupler::ConfigReader::ConfigReader(std::string configFile, std::string participantName)
{

    YAML::Node config = YAML::LoadFile(configFile);

    _preciceConfigFilename = config["precice-config-file"].as<std::string>();

    YAML::Node configInterfaces = config[participantName]["coupled-surfaces"];

    for(int i = 0; i < configInterfaces.size(); i++) {
        struct Interface interface;
        interface.meshName = configInterfaces[i]["mesh-name"].as<std::string>();
        for(int j = 0; j < configInterfaces[i]["patch-names"].size(); j++) {
            interface.patchNames.push_back(configInterfaces[i]["patch-names"][j].as<std::string>());
        }
        if(configInterfaces[i]["write-data"]) {
            for(int j = 0; j < configInterfaces[i]["write-data"].size(); j++) {
                interface.writeData.push_back(configInterfaces[i]["write-data"][j].as<std::string>());
            }
        }
        if(configInterfaces[i]["read-data"]) {
            for(int j = 0; j < configInterfaces[i]["read-data"].size(); j++) {
                interface.readData.push_back(configInterfaces[i]["read-data"][j].as<std::string>());
            }
        }
        // For solvers using the old config file format:
        if(configInterfaces[i]["data"]) {
            for(int j = 0; j < configInterfaces[i]["data"].size(); j++) {
                struct Data data;
                data.name = configInterfaces[i]["data"][j]["name"].as<std::string>();
                data.direction = configInterfaces[i]["data"][j]["direction"].as<std::string>();
                interface.data.push_back(data);
            }
        }
        _interfaces.push_back(interface);
    }

}
