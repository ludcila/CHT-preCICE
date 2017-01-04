/**********************************************************************************************
 *                                                                                            *
 *       CalculiX adapter for heat transfer coupling using preCICE                            *
 *       Developed by Lucía Cheung with the support of SimScale GmbH (www.simscale.com)       *
 *                                                                                            *
 *********************************************************************************************/

#include "ConfigReader.hpp"

void ConfigReader_Read( char * configFilename, char * participantName, char ** preciceConfigFilename, InterfaceConfig ** interfaces, int * numInterface )
{

	YAML::Node config = YAML::LoadFile( configFilename );

	ConfigReader_CheckFields( config, participantName );

	*preciceConfigFilename = strdup( config["precice-config-file"].as<std::string>().c_str() );

	*numInterface = config["participants"][participantName]["interfaces"].size();
	*interfaces = (InterfaceConfig*) malloc( sizeof( InterfaceConfig ) * *numInterface );

	for( int i = 0 ; i < *numInterface ; i++ )
	{

		if( config["participants"][participantName]["interfaces"][i]["nodes-mesh"] )
		{
			( *interfaces )[i].nodesMeshName = strdup( config["participants"][participantName]["interfaces"][i]["nodes-mesh"].as<std::string>().c_str() );
		}
		else
		{
			( *interfaces )[i].nodesMeshName = NULL;
		}

		if( config["participants"][participantName]["interfaces"][i]["faces-mesh"] )
		{
			( *interfaces )[i].facesMeshName = strdup( config["participants"][participantName]["interfaces"][i]["faces-mesh"].as<std::string>().c_str() );
		}
		else
		{
			( *interfaces )[i].facesMeshName = NULL;
		}

		if( config["participants"][participantName]["interfaces"][i]["mesh"] )
		{
			( *interfaces )[i].facesMeshName = strdup( config["participants"][participantName]["interfaces"][i]["mesh"].as<std::string>().c_str() );
		}

		std::string patchName = config["participants"][participantName]["interfaces"][i]["patch"].as<std::string>();
		std::transform( patchName.begin(), patchName.end(), patchName.begin(), toupper );
		( *interfaces )[i].patchName = strdup( patchName.c_str() );

		( *interfaces )[i].numWriteData = config["participants"][participantName]["interfaces"][i]["write-data"].size();
		( *interfaces )[i].numReadData = config["participants"][participantName]["interfaces"][i]["read-data"].size();

		if( ( *interfaces )[i].numWriteData == 0 )
		{
			// write-data is a string
			( *interfaces )[i].numWriteData = 1;
			( *interfaces )[i].writeDataNames = (char**) malloc( sizeof( char* ) * ( *interfaces )[i].numWriteData );
			( *interfaces )[i].writeDataNames[0] = strdup( config["participants"][participantName]["interfaces"][i]["write-data"].as<std::string>().c_str() );
		}
		else
		{
			// write-data is an array
			( *interfaces )[i].writeDataNames = (char**) malloc( sizeof( char* ) * ( *interfaces )[i].numWriteData );

			for( int j = 0 ; j < ( *interfaces )[i].numWriteData ; j++ )
			{
				( *interfaces )[i].writeDataNames[j] = strdup( config["participants"][participantName]["interfaces"][i]["write-data"][j].as<std::string>().c_str() );
			}
		}

		if( ( *interfaces )[i].numReadData == 0 )
		{
			// read-data is a string
			( *interfaces )[i].numReadData = 1;
			( *interfaces )[i].readDataNames = (char**) malloc( sizeof( char* ) * ( *interfaces )[i].numReadData );
			( *interfaces )[i].readDataNames[0] = strdup( config["participants"][participantName]["interfaces"][i]["read-data"].as<std::string>().c_str() );
		}
		else
		{
			// read-data is an array
			( *interfaces )[i].readDataNames = (char**) malloc( sizeof( char* ) * ( *interfaces )[i].numReadData );

			for( int j = 0 ; j < ( *interfaces )[i].numReadData ; j++ )
			{
				( *interfaces )[i].readDataNames[j] = strdup( config["participants"][participantName]["interfaces"][i]["read-data"][j].as<std::string>().c_str() );
			}
		}

	}
}

void ConfigReader_CheckFields( YAML::Node & config, std::string participantName )
{
	if( !config["precice-config-file"] )
	{
		std::cout << "ERROR: precice-config-file not specified in the YAML configuration file." << std::endl;
		exit( 1 );
	}

	if( !config["participants"][participantName] )
	{
		std::cout << "ERROR: Participant '" << participantName << "' is not in the YAML configuration file." << std::endl;
		exit( 1 );
	}

	if( !config["participants"][participantName]["interfaces"] )
	{
		std::cout << "ERROR: Participant '" << participantName << "' does not have interfaces in the YAML configuration file." << std::endl;
		exit( 1 );
	}

	int numInterfaces = config["participants"][participantName]["interfaces"].size();

	for( int i = 0 ; i < numInterfaces ; i++ )
	{
		if( !config["participants"][participantName]["interfaces"][i]["mesh"]
			&& !config["participants"][participantName]["interfaces"][i]["faces-mesh"]
			&& !config["participants"][participantName]["interfaces"][i]["nodes-mesh"] )
		{
			std::cout << "ERROR: Participant '" << participantName << "' does not have valid meshes ('mesh', or 'nodes-mesh' and 'faces-mesh') in the YAML configuration file, for interface #" << i << "." << std::endl;
			exit( 1 );
		}

		if( !config["participants"][participantName]["interfaces"][i]["read-data"] )
		{
			std::cout << "ERROR: Participant '" << participantName << "' does not have 'read-data' in the YAML configuration file, for interface #" << i << "." << std::endl;
			exit( 1 );
		}

		if( !config["participants"][participantName]["interfaces"][i]["write-data"] )
		{
			std::cout << "ERROR: Participant '" << participantName << "' does not have 'write-data' in the YAML configuration file, for interface #" << i << "." << std::endl;
			exit( 1 );
		}

		if( !config["participants"][participantName]["interfaces"][i]["patch"] )
		{
			std::cout << "ERROR: Participant '" << participantName << "' does not have 'patch' in the YAML configuration file, for interface #" << i << "." << std::endl;
			exit( 1 );
		}
	}
}

