#include "PhotonBox/util/FileWatch.h"

#include <thread>

#include "PhotonBox/resource/Shader.h"
#include "PhotonBox/core/ILazyLoadable.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

bool FileWatch::_loading = false;
std::map<std::string, FileWatch::ResourceFile> FileWatch::_watchList;

void FileWatch::addToWatchList(std::string filePath, Shader* shader)
{
	struct stat result;
	std::string s = filePath + ".fs";
	if (stat(s.c_str(), &result) == 0)
	{
		ResourceFile sf = {
			result.st_mtime,
			shader,
		};
		_watchList[s] = sf;
	}
		
	s = filePath + ".vs";
	if (stat(s.c_str(), &result) == 0)
	{
		ResourceFile sf = {
			result.st_mtime,
			shader,
		};
		_watchList[s] = sf;
	}
}

void FileWatch::addToWatchList(std::string filePath, ILazyLoadable* resource)
{
	struct stat result;
	std::string s = filePath;
	if (stat(s.c_str(), &result) == 0)
	{
		ResourceFile sf = {
			result.st_mtime,
			resource,
		};
		_watchList[s] = sf;
	}
}

void FileWatch::checkValidity()
{
	if(!_loading)
		std::thread{ &FileWatch::asyncCheck, this }.detach();
}

void FileWatch::asyncCheck()
{
	_loading = true;
	for (std::map<std::string, ResourceFile>::iterator it = _watchList.begin(); it != _watchList.end(); ++it)
	{
		struct stat result;
		if (stat(it->first.c_str(), &result) == 0 && it->second.stamp != result.st_mtime)
		{
			std::cout << "Updating " << it->first << std::endl;
			it->second.stamp = result.st_mtime;
			it->second.resource->loadAsync();
		}
	}
	_loading = false;
}
