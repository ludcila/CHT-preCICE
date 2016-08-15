#include "ConfigReader.hpp"
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <string.h>

void test() {
	std::cout << "Hey!" << std::endl;
}

void ConfigReader_Read(char * configFilename, char * participantName, char ** preciceConfigFilename, CoupledSurfaceConfig ** coupledSurfaces, int * numCoupledSurfaces) {

	YAML::Node config = YAML::LoadFile(configFilename);

	*preciceConfigFilename = strdup(config["precice-config-file"].as<std::string>().c_str());

	*numCoupledSurfaces = config[participantName]["coupled-surfaces"].size();
	*coupledSurfaces = (CoupledSurfaceConfig*) malloc(sizeof(CoupledSurfaceConfig) * *numCoupledSurfaces);

	for(int i = 0; i < *numCoupledSurfaces; i++) {

		if(config[participantName]["coupled-surfaces"][i]["faces-mesh-name"]) {
			(*coupledSurfaces)[i].facesMeshName = strdup(config[participantName]["coupled-surfaces"][i]["faces-mesh-name"].as<std::string>().c_str());
			(*coupledSurfaces)[i].hasFacesMesh = 1;
		} else {
			(*coupledSurfaces)[i].facesMeshName = NULL;
			(*coupledSurfaces)[i].hasFacesMesh = 0;
		}
		if(config[participantName]["coupled-surfaces"][i]["nodes-mesh-name"]) {
			(*coupledSurfaces)[i].nodesMeshName = strdup(config[participantName]["coupled-surfaces"][i]["nodes-mesh-name"].as<std::string>().c_str());	
			(*coupledSurfaces)[i].hasNodesMesh = 1;
		} else {
			(*coupledSurfaces)[i].nodesMeshName = NULL;
			(*coupledSurfaces)[i].hasNodesMesh = 0;
		}
		(*coupledSurfaces)[i].numPatches = config[participantName]["coupled-surfaces"][i]["patch-names"].size();
		(*coupledSurfaces)[i].patchNames = (char **) malloc(sizeof(char*) * (*coupledSurfaces)[i].numPatches);
		(*coupledSurfaces)[i].numWriteData = config[participantName]["coupled-surfaces"][i]["write-data"].size();
		(*coupledSurfaces)[i].writeData = (char **) malloc(sizeof(char*) * (*coupledSurfaces)[i].numWriteData);
		(*coupledSurfaces)[i].numReadData = config[participantName]["coupled-surfaces"][i]["read-data"].size();
		(*coupledSurfaces)[i].readData = (char **) malloc(sizeof(char*) * (*coupledSurfaces)[i].numReadData);
		
		for(int j = 0; j < (*coupledSurfaces)[i].numPatches; j++) {
			(*coupledSurfaces)[i].patchNames[j] = strdup(config[participantName]["coupled-surfaces"][i]["patch-names"][j].as<std::string>().c_str());
		}
		for(int j = 0; j < (*coupledSurfaces)[i].numWriteData; j++) {
			(*coupledSurfaces)[i].writeData[j] = strdup(config[participantName]["coupled-surfaces"][i]["write-data"][j].as<std::string>().c_str());
		}
		for(int j = 0; j < (*coupledSurfaces)[i].numReadData; j++) {
			(*coupledSurfaces)[i].readData[j] = strdup(config[participantName]["coupled-surfaces"][i]["read-data"][j].as<std::string>().c_str());
		}

	}

}
