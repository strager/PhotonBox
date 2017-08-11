#ifndef RENDERER_H
#define RENDERER_H

class ObjectRenderer;
#include <vector>

class Renderer {
public:
	static bool isRunning() { return _isRunning; }
	static void addToRenderQueue(ObjectRenderer *behaviour);
	static void removeFromRenderQueue(ObjectRenderer *behaviour);
	static void printList();

	void init();
	void render();
	void destroy();
private:
	static bool _isRunning;
	static std::vector<ObjectRenderer*> _renderQueue;
};

#endif // RENDERER_H