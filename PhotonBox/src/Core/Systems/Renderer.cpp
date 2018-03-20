#include <algorithm>
#include <string>
#include <iostream>
#include "../../Components/AmbientLight.h"
#include "../../Components/Camera.h"
#include "../../Components/DirectionalLight.h"
#include "../../Components/MeshRenderer.h"
#include "../../Components/ObjectRenderer.h"
#include "../../Components/PointLight.h"
#include "../../Components/SpotLight.h"
#include "../../Components/Transform.h"
#include "../../Components/TransparentMeshRenderer.h"
#include "../../Resources/DepthShader.h"
#include "../../Resources/GShader.h"
#include "../../Resources/LitShader.h"
#include "../../Resources/Scene.h"
#include "../../Resources/TransparentShader.h"
#include "../../Resources/DeferredShader.cpp"
#include "../DeferredBuffer.h"
#include "../Display.h"
#include "../FrameBuffer.h"
#include "../PostProcessor.h"
#include "Lighting.h"
#include "Renderer.h"
#include "SceneManager.h"

DeferredBuffer Renderer::defBuffer;
bool Renderer::_isDebug;
SkyBox Renderer::_skyBox;
std::vector<ObjectRenderer*> Renderer::_renderListOpaque;
std::vector<ObjectRenderer*> Renderer::_renderListTransparent;
std::map<float, TransparentMeshRenderer*> Renderer::_renderQueueTransparent;
int Renderer::_drawCalls;

ForwardAmbientLightShader* Renderer::_ambientLightShader;
ForwardDirectionalLightShader* Renderer::_directionalLightShader;
ForwardPointLightShader* Renderer::_pointLightShader;
ForwardSpotLightShader* Renderer::_spotLightShader;
TransparentShader* Renderer::_transparentBaseShader;
GShader* Renderer::_gShader;
DeferredShader* Renderer::_deferredShader;
Material* Renderer::_deferredMaterial;

FrameBuffer* Renderer::_mainFrameBuffer;
FrameBuffer* Renderer::_debugFrameBuffer;
Vector3f Renderer::_clearColor = Vector3f(0.3, 0.3, 0.3);


void draw_bbox(Mesh* mesh, Matrix4f modelMatrix);

void Renderer::addToRenderQueue(ObjectRenderer *objectRenderer, bool isOpaque)
{
	if (isOpaque)
	{
		_renderListOpaque.push_back(objectRenderer);
	}
	else
	{
		_renderListTransparent.push_back(objectRenderer);
	}
}

void Renderer::removeFromRenderQueue(ObjectRenderer *objectRenderer)
{
	if (objectRenderer->isOpaque())
	{
		_renderListOpaque.erase(std::remove(_renderListOpaque.begin(), _renderListOpaque.end(), objectRenderer), _renderListOpaque.end());
	}
	else
	{
		_renderListTransparent.erase(std::remove(_renderListTransparent.begin(), _renderListTransparent.end(), objectRenderer), _renderListTransparent.end());
	}
}

void Renderer::setSkyBox(CubeMap* cubeMap)
{
	_skyBox.setLightProbe(new LightProbe(cubeMap));
}

void Renderer::init()
{
	init(1);
}

void Renderer::init(float superSampling)
{
	// OpenGL config
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClearColor(_clearColor.x(), _clearColor.y(), _clearColor.z(), 0);

	_ambientLightShader = ForwardAmbientLightShader::getInstance();
	_directionalLightShader = ForwardDirectionalLightShader::getInstance();
	_pointLightShader = ForwardPointLightShader::getInstance();
	_spotLightShader = ForwardSpotLightShader::getInstance();
	_transparentBaseShader = TransparentShader::getInstance();
	_gShader = GShader::getInstance();
	_deferredShader = DeferredShader::getInstance();
	_mainFrameBuffer = new FrameBuffer(superSampling);
	_mainFrameBuffer->addTextureAttachment("color", true, false);
	_mainFrameBuffer->addDepthBufferAttachment();
	_mainFrameBuffer->ready();
	_debugFrameBuffer = new FrameBuffer(1);
	_debugFrameBuffer->addTextureAttachment("color", false, false);
	_debugFrameBuffer->ready();
	defBuffer.init();
	_isDebug = false;
	_deferredMaterial = new Material(_deferredShader);
	_deferredMaterial->setTexture("gPosition", defBuffer.gBuffer, "gPosition");
	_deferredMaterial->setTexture("gNormal", defBuffer.gBuffer, "gNormal");
	_deferredMaterial->setTexture("gRoughness", defBuffer.gBuffer, "gRoughness");
	_deferredMaterial->setTexture("gMetallic", defBuffer.gBuffer, "gMetallic");
	_deferredMaterial->setTexture("gAlbedo", defBuffer.gBuffer, "gAlbedo");
}

void Renderer::start()
{
	_skyBox.init();

	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		(*it)->init();
	}
	for (std::vector<ObjectRenderer*>::iterator it = _renderListTransparent.begin(); it != _renderListTransparent.end(); ++it)
	{
		(*it)->init();
	}
}

void Renderer::prePass()
{
	defBuffer.gBuffer->enable();
	defBuffer.gBuffer->clear();

	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		if ((*it)->getEnable() && Camera::getMainCamera()->frustumTest(*it))
		{
			glEnable(GL_DEPTH_TEST);
			(*it)->render(_gShader);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::clearDrawCalls()
{
	_drawCalls = 0;
}

void Renderer::render()
{
	render(false);
}

void Renderer::renderShadows(bool captureMode)
{
	glCullFace(GL_FRONT);
	std::vector<DirectionalLight*> directionalLights = Lighting::getLights<DirectionalLight>();
	for (size_t i = 0; i < directionalLights.size(); ++i)
	{
		if (!directionalLights[i]->getEnable()) continue;
		directionalLights[i]->renderShadowMap(captureMode);
	}
	glCullFace(GL_BACK);
}

void Renderer::renderDeferred() {
	renderShadows(false);

	_skyBox.render();

	_mainFrameBuffer->enable();
	_mainFrameBuffer->clear();

	renderBase();

	std::unordered_map<std::type_index, std::vector<LightEmitter*>> lights = Lighting::getAllLights();

	_deferredShader->bind();
	

	int i = -1;
	for (auto const &lightvec : lights)
	{

		for (auto const &light : lightvec.second)
		{
			if (!light->getEnable() || (typeid(*(light->getLightShader())) == typeid(*(ForwardAmbientLightShader::getInstance())))) continue;

			if ((typeid(*(light->getLightShader())) == typeid(*(ForwardDirectionalLightShader::getInstance()))))
			{
				DirectionalLight* dl = dynamic_cast<DirectionalLight*>(light);
				Vector3f lightViewDirection = (Camera::getMainCamera()->getViewMatrix() * Vector4f(dl->direction, 0.0f)).xyz();

				Matrix4f lightView = Matrix4f::lookAt(
					(dl->direction * -3),
					Vector3f(0.0f, 1.0f, 0.0f),
					dl->direction);
				Matrix4f lightSpaceMatrix = dl->lightProjection * lightView;

				++i;
				_deferredShader->setUniform("directionalLights[" + std::to_string(i) + "].lightSpaceMatrix", lightSpaceMatrix);
				_deferredShader->setUniform("directionalLights[" + std::to_string(i) + "].direction", lightViewDirection);
				_deferredShader->setUniform("directionalLights[" + std::to_string(i) + "].color", dl->color);
				_deferredShader->setUniform("directionalLights[" + std::to_string(i) + "].intensity", dl->intensity);


				glActiveTexture(_deferredShader->textures["shadowMap"].unit);
				glBindTexture(GL_TEXTURE_2D, dl->_depthMap);
				//_deferredMaterial->setTexture("shadowMap", dl->_depthMap, "shadowMap");
			}
		}
	}
	
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	defBuffer.gBuffer->render(_deferredMaterial);

	renderTransparents();


	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	


	/*
	int cols = 2;
	int widthX = 0;

	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	//defBuffer.gBuffer->render("gAlbedo");
	defBuffer.gBuffer->render(_deferredMaterial);

	widthX += Display::getWidth() / cols;
	*/

	if (!PostProcessing::isActive())
	{
		FrameBuffer::resetDefaultBuffer();
		_mainFrameBuffer->render("color");
	}
}

void Renderer::render(bool captureMode)
{
	render(captureMode, nullptr);
}

void Renderer::render(bool captureMode, LightMap* lightmap)
{
	if (!captureMode)
	{
		// Bind & clear Shadow FBO
		renderShadows(false);

		// Bind & clear Main FBO
		_mainFrameBuffer->enable();
		_mainFrameBuffer->clear();
	}

	_skyBox.render();
	
	if (captureMode)
	{
		for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
		{
			if ((*it)->getEnable() && Camera::getMainCamera()->frustumTest(*it))
			{
				if (!(*it)->captureble && captureMode) continue;


				if (typeid((**it)) != typeid(MeshRenderer) || ((*it)->getMaterial() != nullptr && (*it)->getMaterial()->shader != nullptr))
				{
					glEnable(GL_DEPTH_TEST);
					(*it)->render();
				}
				else
				{
					glEnable(GL_DEPTH_TEST);


					// IBL
					_ambientLightShader->bind();
					LightProbe* lp = Lighting::findInLightProberVolume((*it)->transform);
					if (!captureMode && lp != nullptr)
					{
						lp->getIrradianceCube()->bind(_ambientLightShader->textures["irradianceMap"].unit);
						lp->getSpecularCube()->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);

						_ambientLightShader->setUniform("minBound", lp->bounds.getMinBoundGlobal());
						_ambientLightShader->setUniform("maxBound", lp->bounds.getMaxBoundGlobal());
						_ambientLightShader->setUniform("boundPos", lp->bounds.getBoundPosition());
						_ambientLightShader->setUniform("useCorrection", true);
					}
					else if (lightmap != nullptr)
					{
						lightmap->irradianceMap->bind(_ambientLightShader->textures["irradianceMap"].unit);
						lightmap->specularMap->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);
						_ambientLightShader->setUniform("useCorrection", false);
					}
					else
					{
						_skyBox.getLightProbe()->getIrradianceCube()->bind(_ambientLightShader->textures["irradianceMap"].unit);
						_skyBox.getLightProbe()->getSpecularCube()->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);
						_ambientLightShader->setUniform("useCorrection", false);
					}

					// AMBIENT
					AmbientLight* ambient = Lighting::getLights<AmbientLight>()[0];
					(*it)->render(_ambientLightShader, ambient);


					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glDepthMask(GL_FALSE);
					glDepthFunc(GL_EQUAL);

					std::unordered_map<std::type_index, std::vector<LightEmitter*>> lights = Lighting::getAllLights();
					for (auto const &lightvec : lights)
					{
						for (auto const &light : lightvec.second)
						{
							if (!light->getEnable() || (typeid(*(light->getLightShader())) == typeid(*(ForwardAmbientLightShader::getInstance())))) continue;
							(*it)->render(light->getLightShader(), light);
						}
					}

					glDepthMask(GL_TRUE);
					glDepthFunc(GL_LESS);
					glDisable(GL_BLEND);

				}
			}
		}
	}
	

	if (!captureMode)
	{
		updateTransparentQueue();
		for (std::map<float, TransparentMeshRenderer*>::reverse_iterator it = _renderQueueTransparent.rbegin(); it != _renderQueueTransparent.rend(); ++it)
		{
			glEnable(GL_DEPTH_TEST);

			// IBL
			_transparentBaseShader->bind();
			LightProbe* lp = Lighting::findInLightProberVolume(it->second->transform);
			if (!captureMode && lp != nullptr)
			{
				lp->getSpecularCube()->bind(_transparentBaseShader->textures["convolutedSpecularMap"].unit);

				_transparentBaseShader->setUniform("minBound", lp->bounds.getMinBoundGlobal());
				_transparentBaseShader->setUniform("maxBound", lp->bounds.getMaxBoundGlobal());
				_transparentBaseShader->setUniform("boundPos", lp->bounds.getBoundPosition());
				_transparentBaseShader->setUniform("useCorrection", true);
			}
			else
			{
				_skyBox.getLightProbe()->getSpecularCube()->bind(_transparentBaseShader->textures["convolutedSpecularMap"].unit);
				_transparentBaseShader->setUniform("useCorrection", false);
			}


			// AMBIENT
			AmbientLight* ambient = Lighting::getLights<AmbientLight>()[0];

			glEnable(GL_BLEND);
			if (it->second->cutout)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glBlendFunc(GL_ONE, GL_ONE);
			}

			it->second->render(_transparentBaseShader, ambient);


			glBlendFunc(GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
			glDepthFunc(GL_EQUAL);

			std::unordered_map<std::type_index, std::vector<LightEmitter*>> lights = Lighting::getAllLights();
			for (auto const &lightvec : lights)
			{
				for (auto const &light : lightvec.second)
				{
					if (!light->getEnable() || (typeid(*(light->getLightShader())) == typeid(*(ForwardAmbientLightShader::getInstance())))) continue;
					it->second->render(light->getLightShader(), light);
				}
			}

			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);

		}
		clearTransparentQueue();
	}


	if (!captureMode && !PostProcessing::isActive())
	{
		FrameBuffer::resetDefaultBuffer();
		_mainFrameBuffer->render("color");
	}
}


void Renderer::renderBase()
{
	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		if ((*it)->getEnable() && Camera::getMainCamera()->frustumTest(*it))
		{
			if (typeid((**it)) != typeid(MeshRenderer) || ((*it)->getMaterial() != nullptr && (*it)->getMaterial()->shader != nullptr))
			{
				glEnable(GL_DEPTH_TEST);
				(*it)->render();
			}
			else
			{
				glEnable(GL_DEPTH_TEST);


				// IBL
				_ambientLightShader->bind();
				LightProbe* lp = Lighting::findInLightProberVolume((*it)->transform);
				if (lp != nullptr)
				{
					lp->getIrradianceCube()->bind(_ambientLightShader->textures["irradianceMap"].unit);
					lp->getSpecularCube()->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);

					_ambientLightShader->setUniform("minBound", lp->bounds.getMinBoundGlobal());
					_ambientLightShader->setUniform("maxBound", lp->bounds.getMaxBoundGlobal());
					_ambientLightShader->setUniform("boundPos", lp->bounds.getBoundPosition());
					_ambientLightShader->setUniform("useCorrection", true);
				}
				else
				{
					_skyBox.getLightProbe()->getIrradianceCube()->bind(_ambientLightShader->textures["irradianceMap"].unit);
					_skyBox.getLightProbe()->getSpecularCube()->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);
					_ambientLightShader->setUniform("useCorrection", false);
				}

				// AMBIENT
				AmbientLight* ambient = Lighting::getLights<AmbientLight>()[0];
				(*it)->render(_ambientLightShader, ambient);
			}
		}
	}

}

void Renderer::renderTransparents()
{
	updateTransparentQueue();
	for (std::map<float, TransparentMeshRenderer*>::reverse_iterator it = _renderQueueTransparent.rbegin(); it != _renderQueueTransparent.rend(); ++it)
	{
		glEnable(GL_DEPTH_TEST);

		// IBL
		_transparentBaseShader->bind();
		LightProbe* lp = Lighting::findInLightProberVolume(it->second->transform);
		if (lp != nullptr)
		{
			lp->getSpecularCube()->bind(_transparentBaseShader->textures["convolutedSpecularMap"].unit);

			_transparentBaseShader->setUniform("minBound", lp->bounds.getMinBoundGlobal());
			_transparentBaseShader->setUniform("maxBound", lp->bounds.getMaxBoundGlobal());
			_transparentBaseShader->setUniform("boundPos", lp->bounds.getBoundPosition());
			_transparentBaseShader->setUniform("useCorrection", true);
		}
		else
		{
			_skyBox.getLightProbe()->getSpecularCube()->bind(_transparentBaseShader->textures["convolutedSpecularMap"].unit);
			_transparentBaseShader->setUniform("useCorrection", false);
		}


		// AMBIENT
		AmbientLight* ambient = Lighting::getLights<AmbientLight>()[0];

		glEnable(GL_BLEND);
		if (it->second->cutout)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glBlendFunc(GL_ONE, GL_ONE);
		}

		it->second->render(_transparentBaseShader, ambient);


		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);

		std::unordered_map<std::type_index, std::vector<LightEmitter*>> lights = Lighting::getAllLights();
		for (auto const &lightvec : lights)
		{
			for (auto const &light : lightvec.second)
			{
				if (!light->getEnable() || (typeid(*(light->getLightShader())) == typeid(*(ForwardAmbientLightShader::getInstance())))) continue;
				it->second->render(light->getLightShader(), light);
			}
		}

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

	}
	clearTransparentQueue();
}


void Renderer::renderAmbient(int pass, LightMap* lightmap, AABB* volume)
{
	_skyBox.render();

	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		if ((*it)->getEnable() && Camera::getMainCamera()->frustumTest(*it))
		{
			if (!(*it)->entity->getStatic()) continue;


			if (typeid((**it)) != typeid(MeshRenderer) || ((*it)->getMaterial() != nullptr && (*it)->getMaterial()->shader != nullptr))
			{
				glEnable(GL_DEPTH_TEST);
				(*it)->render();
			}
			else
			{
				glEnable(GL_DEPTH_TEST);


				// IBL
				_ambientLightShader->bind();
				lightmap->irradianceMap->bind(_ambientLightShader->textures["irradianceMap"].unit);
				lightmap->specularMap->bind(_ambientLightShader->textures["convolutedSpecularMap"].unit);

				if (pass == 0)
				{
					_ambientLightShader->setUniform("useCorrection", false);
				}
				else
				{
					_ambientLightShader->setUniform("minBound", volume->getBoundPosition());
					_ambientLightShader->setUniform("maxBound", volume->getMaxBoundGlobal());
					_ambientLightShader->setUniform("boundPos", volume->getBoundPosition());
					_ambientLightShader->setUniform("useCorrection", true);
				}


				// AMBIENT
				AmbientLight* ambient = Lighting::getLights<AmbientLight>()[0];
				(*it)->render(_ambientLightShader, ambient);


				if (pass == 0)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glDepthMask(GL_FALSE);
					glDepthFunc(GL_EQUAL);

					std::unordered_map<std::type_index, std::vector<LightEmitter*>> lights = Lighting::getAllLights();
					for (auto const &lightvec : lights)
					{
						for (auto const &light : lightvec.second)
						{
							if (!light->getEnable() || (typeid(*(light->getLightShader())) == typeid(*(ForwardAmbientLightShader::getInstance())))) continue;
							(*it)->render(light->getLightShader(), light);
						}
					}

					glDepthMask(GL_TRUE);
					glDepthFunc(GL_LESS);
					glDisable(GL_BLEND);
				}
			}
		}
	}
}

void Renderer::render(Shader* customShader, bool captureMode)
{
	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		if ((*it)->getEnable() && (!(!(*it)->captureble && captureMode)))
		{
			glEnable(GL_DEPTH_TEST);
			std::vector<DirectionalLight*> directionalLights = Lighting::getLights<DirectionalLight>();
			for (size_t i = 0; i < directionalLights.size(); ++i)
			{
				if (!directionalLights[i]->getEnable()) continue;
				(*it)->render(customShader, directionalLights[i]);
			}
		}
	}
}

void Renderer::renderGizmos()
{
	if (_isDebug)
	{
		for (std::vector<Entity*>::iterator it = SceneManager::getCurrentScene()->entityList.begin(); it != SceneManager::getCurrentScene()->entityList.end(); ++it)
		{
			if ((*it)->getEnable())
			{
				(*it)->transform->renderHandels();
			}
		}
	}

	//if (_isDebug)
	{
	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		if ((*it)->getEnable())
		{
			MeshRenderer* mr = (MeshRenderer*)(*it);
			//draw_bbox(mr->getMesh(), mr->transform->getTransformationMatrix());
			
			/*
			Matrix4f projectionMatrix = Camera::getMainCamera()->getProjectionMatrix();
			//glMatrixMode(GL_PROJECTION);
			//glLoadMatrixf((const GLfloat*)&projectionMatrix(0, 0));
			//glMatrixMode(GL_MODELVIEW);
			Matrix4f MVP = Camera::getMainCamera()->getViewProjection() * mr->transform->getTransformationMatrix();
			//glLoadMatrixf((const GLfloat*)&MV(0, 0));

			Vector3f min = (MVP * Vector4f(mr->getMesh()->min, 1.0f)).xyz();
			Vector3f max = (MVP * Vector4f(mr->getMesh()->max, 1.0f)).xyz();

			Vector3f x = min + Vector3f::UNIT_X;
			Vector3f z = min + Vector3f::UNIT_Z;
			Vector3f y = min + Vector3f::UNIT_Y;

			Vector3f negx = (min + Vector3f::UNIT_X) * -1;
			Vector3f negz = (min + Vector3f::UNIT_Z) * -1;
			Vector3f negy = (min + Vector3f::UNIT_Y) * -1;
			*/
			Vector2f min = Camera::getMainCamera()->worldToScreen(mr->getAABB().getMinBoundGlobal());
			Vector2f max = Camera::getMainCamera()->worldToScreen(mr->getAABB().getMaxBoundGlobal());

			glDepthFunc(GL_ALWAYS);

			glUseProgram(0);

			glPointSize(10.0);
			glBegin(GL_POINTS);

			
			glColor3f(0, 0, 1);
			

			glVertex2fv(&min.x());
			//glVertex3fv(&x.x());

			//glVertex3fv(&min.x());
			//glVertex3fv(&z.x());

			//glVertex3fv(&min.x());
			//glVertex3fv(&y.x());

			glColor3f(1, 0, 0);
			glVertex2fv(&max.x());
			//glVertex3fv(&negx.x());

			//glVertex3fv(&max.x());
			//glVertex3fv(&negz.x());

			//glVertex3fv(&max.x());
			//glVertex3fv(&negy.x());

			glEnd();
			glFinish();

			glDepthFunc(GL_LESS);
			
		}
	}
	}



	/*
	int cols = 4;
	int widthX = 0;

	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	defBuffer.gBuffer->render("gAlbedo");
	widthX += Display::getWidth() / cols;

	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	defBuffer.gBuffer->render("gPosition");
	widthX += Display::getWidth() / cols;

	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	defBuffer.gBuffer->render("gNormal");
	widthX += Display::getWidth() / cols;

	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	defBuffer.gBuffer->render("gMetallic");
	widthX += Display::getWidth() / cols;

	
	glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
	defBuffer.gBuffer->render("gRoughness");
	widthX += Display::getWidth() / cols;
	*/

	FrameBuffer::resetDefaultBuffer();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	_debugFrameBuffer->render("color");

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	_debugFrameBuffer->enable();
	_debugFrameBuffer->clear();
	
	
}

void Renderer::updateTransparentQueue()
{
	float bias = 0.0001f;
	for (std::vector<ObjectRenderer*>::iterator it = _renderListTransparent.begin(); it != _renderListTransparent.end(); ++it)
	{
		if ((*it)->getEnable() && (*it)->entity->getStatic() && Camera::getMainCamera()->frustumTest(*it))
		{
			//if (!(*it)->captureVisible && captureMode) continue;
			Vector3f camPos = Camera::getMainCamera()->transform->getPositionWorld();

			float dist = (*it)->transform->getPositionWorld().distanceSqrd(camPos);

			while (1)
			{
				if (_renderQueueTransparent.find(dist) == _renderQueueTransparent.end())
					break;
				else
					dist += bias;
			}

			_renderQueueTransparent[dist] = dynamic_cast<TransparentMeshRenderer*> (*it);
		}
	}
}

void Renderer::clearTransparentQueue()
{
	_renderQueueTransparent.clear();
}

void Renderer::setDebug(bool debug)
{
	_isDebug = debug;
}

void Renderer::destroy()
{
	_renderListOpaque.clear();
}

void Renderer::printList()
{
	for (std::vector<ObjectRenderer*>::iterator it = _renderListOpaque.begin(); it != _renderListOpaque.end(); ++it)
	{
		std::cout << (*it)->getName() << std::endl;
	}
}

void Renderer::setClearColor(Vector3f color)
{
	_clearColor = color;
}

void Renderer::addDrawCall()
{
	++_drawCalls;
}











void draw_bbox(Mesh* mesh, Matrix4f modelMatrix)
{
	LitShader* litShader = LitShader::getInstance();

	// Cube 1x1x1, centered on origin
	GLfloat vertices[] = {
		-0.5, -0.5, -0.5, 1.0,
		0.5, -0.5, -0.5, 1.0,
		0.5,  0.5, -0.5, 1.0,
		-0.5,  0.5, -0.5, 1.0,
		-0.5, -0.5,  0.5, 1.0,
		0.5, -0.5,  0.5, 1.0,
		0.5,  0.5,  0.5, 1.0,
		-0.5,  0.5,  0.5, 1.0,
	};
	GLuint vbo_vertices;
	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLushort elements[] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	GLuint ibo_elements;
	glGenBuffers(1, &ibo_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	Vector3f size = Vector3f(mesh->max.x() - mesh->min.x(), mesh->max.y() - mesh->min.y(), mesh->max.z() - mesh->min.z());
	Vector3f center = Vector3f((mesh->min.x() + mesh->max.x()) / 2, (mesh->min.y() + mesh->max.y()) / 2, (mesh->min.z() + mesh->max.z()) / 2);
	Matrix4f transform = Matrix4f::createTranslation(center) * Matrix4f::createScaling(size);


	litShader->bind();

	
	/* Apply object's transformation matrix */
	Matrix4f m = modelMatrix * transform;
	litShader->setUniform("mvp", m);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

	litShader->enableAttributes();

	/*
	glVertexAttribPointer(
		attribute_v_coord,  // attribute
		4,                  // number of elements per vertex, here (x,y,z,w)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);
	*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	litShader->disableAttributes();

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &ibo_elements);
}