/**********************************************************************************************
 *                                                                                            *
 *       CalculiX adapter for heat transfer coupling using preCICE                            *
 *       Developed by Lucía Cheung with the support of SimScale GmbH (www.simscale.com)       *
 *                                                                                            *
 *********************************************************************************************/

#ifndef CONFIGREADER_H
#define CONFIGREADER_H

typedef struct InterfaceConfig {
	char * facesMeshName;
	char * nodesMeshName;
	char * patchName;
	int numWriteData;
	int numReadData;
	char ** writeDataNames;
	char ** readDataNames;
} InterfaceConfig;

void ConfigReader_Read(char * configFilename, char * participantName, char ** preciceConfigFilename, InterfaceConfig ** interfaces, int * numInterfaces);


#endif
