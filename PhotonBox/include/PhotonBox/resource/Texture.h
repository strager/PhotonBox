#ifndef TEXTURE_H
#define TEXTURE_H

class Scene;

#include <string>

#include "PhotonBox/core/ManagedResource.h"
#include "PhotonBox/core/ILazyLoadable.h"
#include "PhotonBox/core/OpenGL.h"

class Texture : public ManagedResource, public ILazyLoadable
{
public:
	//Texture(bool generateMipMaps, bool hdr = false);
	Texture(std::string fileName, bool generateMipMaps = false, bool hdr = false);
	~Texture();

	void bind();
	void bind(GLuint textureUnit);
	int getWidth() { return _width; }
	int getHeight() { return _height; }
	bool isHDR() { return _isHDR; }
	std::string getName() { return _fileName; }
	GLuint getTextureLocation() { return _texture; }

	static unsigned char* loadIcon(const std::string& fileName, int& width, int& height);
	static void freeIcon(unsigned char* data);

	void sendToGPU() override;
private:
	friend class Scene;
	bool _isHDR, _isMipMap;
	GLuint _texture;
	int _width, _height;
	std::string _fileName;
	unsigned char* _data = NULL;

	void loadFromFile() override;
	void blankInitialize();
};

#endif // TEXTURE_H
