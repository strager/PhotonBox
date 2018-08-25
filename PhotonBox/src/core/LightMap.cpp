#include "PhotonBox/core/LightMap.h"

#ifdef MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

LightMap::LightMap(const std::vector<std::string>& allFaces)
{
	enviromentMap = new CubeMap(allFaces);
	generateLightMaps();
}

LightMap::LightMap(CubeMap* cubemap)
{
	enviromentMap = cubemap;
	generateLightMaps();
}

LightMap::~LightMap()
{
	delete enviromentMap;
	delete irradianceMap;
	delete specularMap;
}

void LightMap::generateLightMaps()
{
	irradianceMap = new CubeMap(32);
	irradianceMap->generateIrradiance(enviromentMap->getLocation());

	specularMap = new CubeMap(128, true);
	specularMap->generateSpecularConvolution(enviromentMap->getLocation());
}
