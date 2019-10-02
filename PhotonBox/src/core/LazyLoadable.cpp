#include "PhotonBox/core/LazyLoadable.h"

#include <thread>
#include <iostream>

#include "PhotonBox/core/system/ResourceManager.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

void LazyLoadable::loadAsync()
{
	_isLoaded = false;
	_isInitialized = false;
	
	blankInitialize();

	ResourceManager::addToInitializationList(this);
	std::thread{ &LazyLoadable::load, this }.detach();
}

void LazyLoadable::forceLoad()
{
	blankInitialize();
	load();
	initialize();
}

void LazyLoadable::setRogue()
{
	_rougue = true;
}

void LazyLoadable::initialize()
{
	submitBuffer();
	_isInitialized = true;

}

bool LazyLoadable:: isLoaded()
{
	return _isLoaded; 
}

bool LazyLoadable::isInitialized()
{
	return _isInitialized; 
}

std::string LazyLoadable::getFilePath()
{
	return _filePath;
}

void LazyLoadable::load()
{
	loadFromFile();
	_isLoaded = true;

	if (_rougue) {
		delete this;
	}
}