#ifndef CORE_H
#define CORE_H

#include <string>

class Display;
class SceneManager;
class Renderer;
class UIRenderer;
class Time;
class Logic;
class Physics;
class PostProcessing;
class Lighting;
class InputManager;
class Config;

class Core
{
public:
	void init();
	void start();
	void reset();
	void run();
	void destroy();

	static bool isRunning() { return _isRunning; }
	static void stop();
private:
	Time * _time;
	Display* _display;
	InputManager* _inputManager;
	UIRenderer* _uiRenderer;
	SceneManager* _sceneManager;
	Renderer* _renderer;
	Logic* _logic;
	Physics* _physics;
	PostProcessing* _postPocessing;
	Lighting* _lighting;

	Config* _config;

	const double FIXED_TIME_INTERVAL = 30;

	double _accumulatedTime, _newTime, _lastTime;
	int _fps = 0;

	static bool _isRunning;
};

#endif // CORE_H