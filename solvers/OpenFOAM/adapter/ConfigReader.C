#include "ConfigReader.h"


void ofcoupler::ConfigReader::checkFields(std::string filename, YAML::Node & config, std::string participantName)
{
    if(!config["precice-config-file"]) {
        std::cerr << "ERROR in " << filename << ": precice-config-file not specified\n";
        exit(1);
    }
    if(!config[participantName]) {
        std::cerr << "ERROR in " << filename << ": participant " << participantName << " not found in the YAML config file\n";
        exit(1);
    } else {
        if(!config[participantName]["coupled-surfaces"]) {
            std::cerr << "ERROR in " << filename << ": coupled-surfaces not specified for participant\n" << participantName;
            exit(1);
        } else {
            for(int i = 0; i < config[participantName]["coupled-surfaces"].size(); i++) {
                if(!config[participantName]["coupled-surfaces"][i]["mesh-name"]) {
                    std::cerr << "ERROR in " << filename << ": mesh-name not specified\n";
                    exit(1);
                }
                if(!config[participantName]["coupled-surfaces"][i]["patch-names"]) {
                    std::cerr << "ERROR in " << filename << ": patch-names not specified\n";
                    exit(1);
                }
                if(!config[participantName]["coupled-surfaces"][i]["write-data"]) {
                    std::cerr << "ERROR in " << filename << ": write-data not specified\n";
                    exit(1);
                }
                if(!config[participantName]["coupled-surfaces"][i]["read-data"]) {
                    std::cerr << "ERROR in " << filename << ": read-data not specified\n";
                    exit(1);
                }
            }
        }
    }
}

ofcoupler::ConfigReader::ConfigReader(std::string configFile, std::string participantName)
{

    YAML::Node config = YAML::LoadFile(configFile);
    
    checkFields(configFile, config, participantName);

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
        _interfaces.push_back(interface);
    }

}
