#include "Config.h"

#include <fstream>
#include <vector>
#include <iostream>
#include "../Core/Util.h"

Config::Profile Config::profile;

void Config::readConfig()
{
	std::ifstream file(FILE_PATH);
	std::string line;
	std::vector<std::string> tokens;

	while (std::getline(file, line)) {
		tokens.clear();
		Util::split(line, "=", tokens);

		if (tokens[0].compare("width") == 0) 
		{
			profile.width = std::stoi(tokens[1]);
		}
		else if (tokens[0].compare("height") == 0)
		{
			profile.height = std::stoi(tokens[1]);
		}
		else if (tokens[0].compare("fullscreen") == 0)
		{
			profile.fullscreen = (bool) std::stoi(tokens[1]);
		}
		else if (tokens[0].compare("vsync") == 0)
		{
			profile.vsync = (bool) std::stoi(tokens[1]);
		}
		else if (tokens[0].compare("supersampling") == 0)
		{
			profile.supersampling = (bool) std::stoi(tokens[1]);
		}
		else
		{
			std::cout << "Unknown token: " << tokens[0] << std::endl;
		}
	}
}
