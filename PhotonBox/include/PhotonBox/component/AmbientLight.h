#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H

#include "PhotonBox/component/LightEmitter.h"

class AmbientLight : public LightEmitter
{
public:
	void init() override;
	void destroy() override;
	Shader* getLightShader() override;
};

#endif // AMBIENT_LIGHT_H
