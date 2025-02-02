#include "PhotonBox/core/system/ResourceManager.h"

#include <chrono>
#include <iostream>

#include "PhotonBox/core/ILazyLoadable.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

std::vector<ILazyLoadable*> ResourceManager::_initializationList;

bool ResourceManager::allReady()
{
	return _initializationList.size() == 0;
}

void ResourceManager::lazyLoad(bool force)
{
	if (allReady()) { 
		return;
	}

	auto start = std::chrono::system_clock::now();
	for (int i = static_cast<int>(_initializationList.size()-1); i >= 0; --i)
	{
		if (_initializationList[i]->isLoaded()) {
			_initializationList[i]->initialize();

			_initializationList.erase(_initializationList.begin() + i);

			auto check = std::chrono::system_clock::now();
			if ((check - start).count() > 30000000 && !force) {
				std::cout << "Delaying initialization to next frame\n";
				return;
			}
		}
	}
}

void ResourceManager::addToInitializationList(ILazyLoadable * resource)
{
	_initializationList.push_back(resource);
}

void ResourceManager::reset()
{
	_initializationList.clear();
}
