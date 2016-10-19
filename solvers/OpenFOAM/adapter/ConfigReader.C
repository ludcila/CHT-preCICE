#include "ConfigReader.h"

void adapter::ConfigReader::checkFields( std::string filename, YAML::Node & config, std::string participantName )
{

	if( !config["precice-config-file"] )
	{
		BOOST_LOG_TRIVIAL( error ) << "precice-config-file not specified in " << filename;
		exit( 1 );
	}

	if( !config["participants"][participantName] )
	{
		std::cerr << "ERROR in " << filename << ": participant " << participantName << " not found in the YAML config file\n";
		exit( 1 );
	}
	else
	{
		if( !config["participants"][participantName]["interfaces"] )
		{
			std::cerr << "ERROR in " << filename << ": interfaces not specified for participant\n" << participantName;
			exit( 1 );
		}
		else
		{
			for( uint i = 0 ; i < config["participants"][participantName]["interfaces"].size() ; i++ )
			{
				if( !config["participants"][participantName]["interfaces"][i]["mesh"] )
				{
					std::cerr << "ERROR in " << filename << ": mesh not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["patches"] )
				{
					std::cerr << "ERROR in " << filename << ": patches not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["write-data"] )
				{
					std::cerr << "ERROR in " << filename << ": write-data not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["read-data"] )
				{
					std::cerr << "ERROR in " << filename << ": read-data not specified\n";
					exit( 1 );
				}
			}
		}
	}
}

adapter::ConfigReader::ConfigReader( std::string configFile, std::string participantName )
{

	YAML::Node config = YAML::LoadFile( configFile );

	checkFields( configFile, config, participantName );

	_preciceConfigFilename = config["precice-config-file"].as<std::string>();

	YAML::Node configInterfaces = config["participants"][participantName]["interfaces"];

	for( uint i = 0 ; i < configInterfaces.size() ; i++ )
	{
		struct Interface interface;
		interface.meshName = configInterfaces[i]["mesh"].as<std::string>();

		for( uint j = 0 ; j < configInterfaces[i]["patches"].size() ; j++ )
		{
			interface.patchNames.push_back( configInterfaces[i]["patches"][j].as<std::string>() );
		}

		if( configInterfaces[i]["write-data"] )
		{
			if( configInterfaces[i]["write-data"].size() > 0 )
			{
				// write-data is an array
				for( uint j = 0 ; j < configInterfaces[i]["write-data"].size() ; j++ )
				{
					interface.writeData.push_back( configInterfaces[i]["write-data"][j].as<std::string>() );
				}
			}
			else
			{
				// write-data is a string
				interface.writeData.push_back( configInterfaces[i]["write-data"].as<std::string>() );
			}
		}

		if( configInterfaces[i]["read-data"] )
		{
			if( configInterfaces[i]["read-data"].size() > 0 )
			{
				// read-data is an array
				for( uint j = 0 ; j < configInterfaces[i]["read-data"].size() ; j++ )
				{
					interface.readData.push_back( configInterfaces[i]["read-data"][j].as<std::string>() );
				}
			}
			else
			{
				// read-data is a string
				interface.readData.push_back( configInterfaces[i]["read-data"].as<std::string>() );
			}
		}
		_interfaces.push_back( interface );
	}
}

