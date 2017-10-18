#ifndef RENDERER_H
#define RENDERER_H

class ObjectRenderer;
class CubeMap;

#include <vector>
#include "../../Resources/SkyBox.h"
#include "../../Resources/ForwardAmbientLightShader.h"
#include "../../Resources/ForwardDirectionalLightShader.h"
#include "../../Resources/ForwardPointLightShader.h"

class Renderer {
public:
	static bool isDebug() { return _isDebug; }
	static void setDebug(bool debug);
	static void addToRenderQueue(ObjectRenderer *behaviour);
	static void removeFromRenderQueue(ObjectRenderer *behaviour);
	static void setSkyBox(CubeMap* cubeMap);
	static SkyBox* getSkyBox() { return &_skyBox; }
	static void printList();

	void init();
	void start();
	void render();
	void destroy();
private:
	static SkyBox _skyBox;
	static bool _isDebug;
	static std::vector<ObjectRenderer*> _renderQueue;

	static ForwardAmbientLightShader* _ambientLightShader;
	static ForwardDirectionalLightShader* _directionalLightShader;
	static ForwardPointLightShader* _pointLightShader;
};

#endif // RENDERER_H
