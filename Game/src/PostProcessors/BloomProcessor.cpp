#ifndef BLOOM_PROCESSOR_CPP
#define BLOOM_PROCESSOR_CPP

#include <resource/PostProcessor.h>
#include <resource/Material.h>
#include <core/system/Renderer.h>
#include <core/Display.h>

#include "../Shader/AddShader.cpp"
#include "../Shader/BlurHShader.cpp"
#include "../Shader/BlurVShader.cpp"
#include "../Shader/CutOffShader.cpp"

#ifdef PB_MEM_DEBUG
#include "PhotonBox/util/MEMDebug.h"
#define new DEBUG_NEW
#endif

class BloomProcessor : public PostProcessor
{
public:
	Material * m_cutOff;

	Material* m_blur_h;
	Material* m_blur_v;

	FrameBuffer* fb_cutOff_2;
	FrameBuffer* fb_cutOff_4;
	FrameBuffer* fb_cutOff_8;
	FrameBuffer* fb_cutOff_16;

	FrameBuffer* fb_blur_h_2;
	FrameBuffer* fb_blur_v_2;
	FrameBuffer* fb_blur_h_4;
	FrameBuffer* fb_blur_v_4;
	FrameBuffer* fb_blur_h_8;
	FrameBuffer* fb_blur_v_8;
	FrameBuffer* fb_blur_h_16;
	FrameBuffer* fb_blur_v_16;

	bool debug = false;


	BloomProcessor(int index) : PostProcessor(index)
	{
		fb_cutOff_2 = new FrameBuffer(0.5f);
		fb_cutOff_2->addTextureAttachment("color");
		fb_cutOff_2->ready();
		fb_cutOff_4 = new FrameBuffer(0.24f);
		fb_cutOff_4->addTextureAttachment("color");
		fb_cutOff_4->ready();
		fb_cutOff_8 = new FrameBuffer(0.125f);
		fb_cutOff_8->addTextureAttachment("color");
		fb_cutOff_8->ready();
		fb_cutOff_16 = new FrameBuffer(0.0625f);
		fb_cutOff_16->addTextureAttachment("color");
		fb_cutOff_16->ready();

		fb_blur_h_2 = new FrameBuffer(0.5f);
		fb_blur_h_2->addTextureAttachment("color");
		fb_blur_h_2->ready();
		fb_blur_v_2 = new FrameBuffer(0.5f);
		fb_blur_v_2->addTextureAttachment("color");
		fb_blur_v_2->ready();
		fb_blur_h_4 = new FrameBuffer(0.25f);
		fb_blur_h_4->addTextureAttachment("color");
		fb_blur_h_4->ready();
		fb_blur_v_4 = new FrameBuffer(0.25f);
		fb_blur_v_4->addTextureAttachment("color");
		fb_blur_v_4->ready();
		fb_blur_h_8 = new FrameBuffer(0.125f);
		fb_blur_h_8->addTextureAttachment("color");
		fb_blur_h_8->ready();
		fb_blur_v_8 = new FrameBuffer(0.125f);
		fb_blur_v_8->addTextureAttachment("color");
		fb_blur_v_8->ready();
		fb_blur_h_16 = new FrameBuffer(0.0625f);
		fb_blur_h_16->addTextureAttachment("color");
		fb_blur_h_16->ready();
		fb_blur_v_16 = new FrameBuffer(0.0625f);
		fb_blur_v_16->addTextureAttachment("color");
		fb_blur_v_16->ready();


		m_cutOff = new Material(CutOffShader::getInstance());
		m_cutOff->setProperty<float>("threshold", 10.0f);
		m_cutOff->setTexture("renderTexture", mainBuffer, "color");

		m_blur_h = new Material(BlurHShader::getInstance());
		m_blur_v = new Material(BlurVShader::getInstance());
	}

	void render(FrameBuffer* nextBuffer) override
	{
		// Blur 16
		fb_cutOff_16->enable();
		mainBuffer->render(m_cutOff);

		m_blur_h->setProperty("offset", (1.0f / fb_cutOff_16->getWidth()));
		m_blur_h->setTexture("renderTexture", fb_cutOff_16, "color");
		fb_blur_h_16->enable();
		fb_cutOff_16->render(m_blur_h);

		m_blur_v->setProperty("offset", (1.0f / fb_blur_h_16->getHeight()));
		m_blur_v->setTexture("renderTexture", fb_blur_h_16, "color");
		fb_blur_v_16->enable();
		fb_blur_h_16->render(m_blur_v);

		int cols = 4;
		int widthX = 0;

		if (debug)
		{
			Renderer::getDebugBuffer()->enable();
			glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
			fb_blur_v_16->render("color");
			widthX += Display::getWidth() / cols;
		}


		// Blur 8
		fb_cutOff_8->enable();
		mainBuffer->render(m_cutOff);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		fb_blur_v_16->render("color");
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		m_blur_h->setProperty("offset", (1.0f / fb_cutOff_8->getWidth()));
		m_blur_h->setTexture("renderTexture", fb_cutOff_8, "color");
		fb_blur_h_8->enable();
		fb_cutOff_8->render(m_blur_h);

		m_blur_v->setProperty("offset", (1.0f / fb_blur_h_8->getHeight()));
		m_blur_v->setTexture("renderTexture", fb_blur_h_8, "color");
		fb_blur_v_8->enable();
		fb_blur_h_8->render(m_blur_v);

		if (debug)
		{
			Renderer::getDebugBuffer()->enable();
			glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
			fb_blur_v_8->render("color");
			widthX += Display::getWidth() / cols;
		}


		// Blur 4
		fb_cutOff_4->enable();
		mainBuffer->render(m_cutOff);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		fb_blur_v_8->render("color");
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		m_blur_h->setProperty("offset", (1.0f / fb_cutOff_4->getWidth()));
		m_blur_h->setTexture("renderTexture", fb_cutOff_4, "color");
		fb_blur_h_4->enable();
		fb_cutOff_4->render(m_blur_h);

		m_blur_v->setProperty("offset", (1.0f / fb_blur_h_4->getHeight()));
		m_blur_v->setTexture("renderTexture", fb_blur_h_4, "color");
		fb_blur_v_4->enable();
		fb_blur_h_4->render(m_blur_v);

		if (debug)
		{
			Renderer::getDebugBuffer()->enable();
			glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
			fb_blur_v_4->render("color");
			widthX += Display::getWidth() / cols;
		}


		// Blur 2
		fb_cutOff_2->enable();
		mainBuffer->render(m_cutOff);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		fb_blur_v_4->render("color");
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		m_blur_h->setProperty("offset", (1.0f / fb_cutOff_2->getWidth()));
		m_blur_h->setTexture("renderTexture", fb_cutOff_2, "color");
		fb_blur_h_2->enable();
		fb_cutOff_2->render(m_blur_h);

		m_blur_v->setProperty("offset", (1.0f / fb_blur_h_2->getHeight()));
		m_blur_v->setTexture("renderTexture", fb_blur_h_2, "color");
		fb_blur_v_2->enable();
		fb_blur_h_2->render(m_blur_v);

		if (debug)
		{
			Renderer::getDebugBuffer()->enable();
			glViewport(widthX, 0, Display::getWidth() / cols, Display::getHeight() / cols);
			fb_blur_v_2->render("color");
			widthX += Display::getWidth() / cols;
		}

		// Render
		nextBuffer->enable();
		mainBuffer->render("color");
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		fb_blur_v_2->render("color");
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}

	void destroy() override
	{
		delete m_cutOff;
		delete m_blur_h;
		delete m_blur_v;

		delete fb_cutOff_2;
		delete fb_cutOff_4;
		delete fb_cutOff_8;
		delete fb_cutOff_16;

		delete fb_blur_h_2;
		delete fb_blur_v_2;
		delete fb_blur_h_4;
		delete fb_blur_v_4;
		delete fb_blur_h_8;
		delete fb_blur_v_8;
		delete fb_blur_h_16;
		delete fb_blur_v_16;
	}
};

#endif // BLOOM_PROCESSOR_CPP