/**********************************************************************************************
 *                                                                                            *
 *       CalculiX adapter for heat transfer coupling using preCICE                            *
 *       Developed by Lucía Cheung with the support of SimScale GmbH (www.simscale.com)       *
 *                                                                                            *
 *********************************************************************************************/

#ifndef CONFIGREADER_HPP
#define CONFIGREADER_HPP


#include "yaml-cpp/yaml.h"
#include <iostream>
#include <algorithm>
#include <string.h>

extern "C" {

	#include "ConfigReader.h"

}

void ConfigReader_CheckFields( YAML::Node & config, std::string participantName );

#endif
