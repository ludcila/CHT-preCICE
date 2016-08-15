#ifndef CONFIGREADER_H
#define CONFIGREADER_H

void test();

typedef struct CouplingDataConfig {
	char * name;
	char * direction;
} CouplingDataConfig;

typedef struct CoupledSurfaceConfig {
	char * facesMeshName;
	char * nodesMeshName;
	int hasFacesMesh;
	int hasNodesMesh;
	char * meshType;
	int numPatches;
	char ** patchNames;
	char ** writeData;
	char ** readData;
	int numWriteData;
	int numReadData;
	int numData;
	struct CouplingDataConfig * data;
	char * writeDataName;
	char * readDataName;
	int isDirichletBC;
} CoupledSurfaceConfig;

void ConfigReader_Read(char * configFilename, char * participantName, char ** preciceConfigFilename, CoupledSurfaceConfig ** coupledSurfaces, int * numCoupledSurfaces);


#endif
