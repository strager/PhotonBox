#include "PhotonBox/core/Display.h"

#include "PhotonBox/component/Camera.h"
#include "PhotonBox/resource/Texture.h"
#include "PhotonBox/resource/FrameBuffer.h"
#include "PhotonBox/resource/PostProcessor.h"
#include "PhotonBox/core/Resources.h"
#include "PhotonBox/core/system/PostProcessing.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

bool Display::_isRunning;
int Display::_width, Display::_height;
GLFWwindow* Display::_window;
bool Display::_isVSync;

void window_size_callback(GLFWwindow*, int, int);

void Display::init(const std::string& title, unsigned int width, unsigned int height, bool fullscreen, bool vsync)
{
	_width = width;
	_height = height;

	if (!glfwInit())
	{
		std::cerr << "ERROR: Failed to initialize GLFW.\n";
		exit(EXIT_FAILURE);
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	_window = glfwCreateWindow(width, height, title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!_window)
	{
		std::cerr << "ERROR: Failed to create a window. (OpenGL version 4.4 required)\n";
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// Set Icon
	GLFWimage icon[1];
	icon[0].pixels = Texture::loadIcon(Resources::ENGINE_RESOURCES + "/logo.png", icon[0].width, icon[0].height);
	glfwSetWindowIcon(_window, 1, icon);
	Texture::freeIcon(icon[0].pixels);

	setVSync(vsync);
	_isRunning = true;

	std::cout << "OpenGL Version " << glGetString(GL_VERSION) << std::endl << std::endl;

	glfwSetWindowSizeCallback(_window, window_size_callback);
}

void Display::destroy()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Display::setVSync(bool vsync)
{
	glfwSwapInterval((int)vsync);
	_isVSync = vsync;
}

void Display::setTitle(const std::string& title)
{
	glfwSetWindowTitle(_window, title.c_str());
}

void Display::swapBuffer()
{
	glfwSwapBuffers(_window);

	_isRunning = !glfwWindowShouldClose(_window);
}

unsigned int Display::getWidth()
{
	return _width;
}

unsigned int Display::getHeight()
{
	return _height;
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) return;

	Display::setRect(width, height);
	Camera::getMainCamera()->updateAspect();
	FrameBuffer::resizeAll();
	PostProcessing::resizeAll();
}
