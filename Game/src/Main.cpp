#include <iostream>
#include <map>

#include <core/Core.h>
#include <resource/Scene.h>

#include "Scenes/ShelbyScene.cpp"
#include "Scenes/MaterialTestScene.cpp"
#include "Scenes/PhysicsScene.cpp"
#include "../../test/MathTest.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

bool temp = false;

int main(void)
{
#ifdef PB_MEM_DEBUG
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#elif _DEBUG
	// Tests
	MathTest::startTest();
#else
#ifdef PB_PLATFORM_WIN
	FreeConsole();
#endif
#endif

	// Create Scenes
	std::map<std::string, Scene*> sceneMap;
	sceneMap["1. Material Test"] = new MaterialTestScene();
	sceneMap["2. Shelby Scene"] = new ShelbyScene(); 
	sceneMap["3. Physics Scene"] = new PhysicsScene();

	// Create and initialize the Core
	Core core;
	core.init(sceneMap);
	core.run();
	core.destroy();

#ifdef _DEBUG
	std::cin.ignore();
#endif

	return 0;
}