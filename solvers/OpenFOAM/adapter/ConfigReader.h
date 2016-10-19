#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>
#include <boost/log/trivial.hpp>
#include "yaml-cpp/yaml.h"
#include "Adapter.h"

namespace adapter
{

class ConfigReader
{

	struct Data {
		std::string direction;
		std::string name;
	};

	struct Interface {
		std::string meshName;
		std::vector<std::string> patchNames;
		std::vector<std::string> writeData;
		std::vector<std::string> readData;
		std::vector<struct Data> data;
	};

protected:

	YAML::Node _config;
	std::vector<struct Interface> _interfaces;
	std::string _preciceConfigFilename;
	void checkFields( std::string filename, YAML::Node & config, std::string participantName );

public:

	ConfigReader( std::string configFile, std::string participantName );
	std::vector<struct Interface> interfaces()
	{
		return _interfaces;
	}

	std::string preciceConfigFilename()
	{
		return _preciceConfigFilename;
	}

};

}

#endif // CONFIGREADER_H
