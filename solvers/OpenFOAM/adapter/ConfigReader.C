#include "ConfigReader.h"

void adapter::ConfigReader::checkFields( std::string filename, YAML::Node & config, std::string participantName )
{

	if( !config["precice-config-file"] )
	{
		BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": precice-config-file not specified";
		exit( 1 );
	}

	if( !config["participants"][participantName] )
	{
		BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": participant \"" << participantName << "\" not specified";
		exit( 1 );
	}
	else
	{
		if( !config["participants"][participantName]["interfaces"] )
		{
			BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": interfaces not specified for participant \"" << participantName << "\"";
			exit( 1 );
		}
		else
		{
			for( uint i = 0 ; i < config["participants"][participantName]["interfaces"].size() ; i++ )
			{
				if( !config["participants"][participantName]["interfaces"][i]["mesh"] )
				{
					BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": mesh not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["patches"] )
				{
					BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": patches not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["write-data"] )
				{
					BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": write-data not specified\n";
					exit( 1 );
				}

				if( !config["participants"][participantName]["interfaces"][i]["read-data"] )
				{
					BOOST_LOG_TRIVIAL( error ) << "ERROR in " << filename << ": read-data not specified\n";
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

