#ifndef ADD_SHADER_CPP
#define ADD_SHADER_CPP

#include <resource/Shader.h>

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

class AddShader : public InstancedShader<AddShader>
{
public:
	std::string getFilePath() override
	{
		return std::string("./res/shaders/post-processing/add");
	}

	void addAttributes() override
	{
		addAttribut("position", Vertex::AttibLocation::POSITION);
	}
};

#endif // ADD_SHADER_CPP