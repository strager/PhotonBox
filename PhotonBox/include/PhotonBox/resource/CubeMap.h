#ifndef CUBE_MAP_H
#define CUBE_MAP_H

class Mesh;

#include <vector>
#include <string>

#include "PhotonBox/core/ManagedResource.h"
#include "PhotonBox/core/OpenGL.h"

class CubeMap : public ManagedResource
{
public:
	enum Face
	{
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		BACK,
		FRONT
	};

	CubeMap(const std::vector<std::string>& allFaces);
	CubeMap(int resolution);
	CubeMap(int resolution, bool mipMaps);
	~CubeMap();
	void generateIrradiance(GLuint map);
	void generateSpecularConvolution(GLuint map);
	GLuint getLocation() { return _cubeMap; }
	void bind();
	void bind(GLenum textureUnit);
	int getWidth() { return _width; }
	int getHeight() { return _height; }
	bool hasMipMaps() { return _isMip; }
private:
	void loadCubeMap(const std::vector<std::string>& faces);
	GLuint _cubeMap;
	int _width, _height;
	bool _isMip;

	void renderCube();
	GLuint _captureFBO, _captureRBO;
	Mesh* _mesh = nullptr;
};

#endif // CUBE_MAP_H
