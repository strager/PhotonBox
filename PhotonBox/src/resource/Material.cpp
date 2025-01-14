#include "PhotonBox/resource/Material.h"

#include "PhotonBox/resource/FrameBuffer.h"
#include "PhotonBox/resource/CubeMap.h"
#include "PhotonBox/resource/Shader.h"
#include "PhotonBox/resource/Texture.h"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

Material::~Material()
{
	for (std::unordered_map<std::string, SuperObject*>::iterator it = _uniformMap.begin(); it != _uniformMap.end(); ++it)
	{
		delete it->second;
	}
	_uniformMap.clear();
}

void Material::setTexture(const std::string & uniformName, Texture* texture)
{
	_textreMap[uniformName] = texture;
}

void Material::setTexture(const std::string & uniformName, FrameBuffer * buffer, std::string attachmentName)
{
	_frameBufferMap[uniformName] = buffer->getAttachment(attachmentName);
}

void Material::setCubeMap(const std::string & uniformName, CubeMap* cubeMap)
{
	_cubeMapMap[uniformName] = cubeMap;
}

void Material::updateUniforms()
{
	updateUniforms(shader);
}

void Material::updateUniforms(Shader* shader)
{
	for (std::unordered_map<std::string, SuperObject*>::const_iterator it = _uniformMap.begin(); it != _uniformMap.end(); ++it)
	{
		it->second->update(shader);
	}
}

void Material::bindTextures()
{
	bindTextures(shader);
}

void Material::bindTextures(Shader* shader)
{
	// Textures
	for (std::unordered_map<std::string, Texture*>::const_iterator it = _textreMap.begin(); it != _textreMap.end(); ++it)
	{
		if (shader->textures.find(it->first) != shader->textures.end())
			it->second->bind(shader->textures[it->first].unit);
	}

	// Framebuffers
	for (std::unordered_map<std::string, FrameBuffer::BufferAttachment*>::const_iterator it = _frameBufferMap.begin(); it != _frameBufferMap.end(); ++it)
	{
		if (shader->textures.find(it->first) != shader->textures.end())
			it->second->frameBuffer->bind(shader->textures[it->first].unit, it->second->name);
	}

	// CubeMaps
	for (std::unordered_map<std::string, CubeMap*>::const_iterator it = _cubeMapMap.begin(); it != _cubeMapMap.end(); ++it)
	{
		if (shader->textures.find(it->first) != shader->textures.end())
			it->second->bind(shader->textures[it->first].unit);
	}
}

Texture* Material::getTexture(const std::string & uniformName)
{
	return _textreMap[uniformName];
}

CubeMap* Material::getCubeMap(const std::string & uniformName)
{
	return _cubeMapMap[uniformName];
}
