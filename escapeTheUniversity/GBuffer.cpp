#include <GL/glew.h>
#include "GBuffer.hpp"
#include "Initialization.hpp"
#include "./Debug/Debugger.hpp"

/*Gbuffer used for deferred shading.*/
GBuffer::GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	glGenTextures(4, positionNormalColorHandles);

	// Normals, albedo color, and material index in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[0],positionNormalColorHandles[0], 0);

	// World space coords and specular color in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[1], positionNormalColorHandles[1], 0);

	// Final, the one rendered first and blitted last
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[2], positionNormalColorHandles[2], 0);

	// Depth and stencil buffer
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[3]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH32F_STENCIL8,	MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,positionNormalColorHandles[3], 0);
	

	Debugger::getInstance()->checkWholeFramebufferCompleteness();

	glDrawBuffers(deferredShadingColorTextureCount, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glBindVertexArray(0);
}

/*At the start of each frame we need to clear the final texture which is attached to attachment point number 2.*/
void GBuffer::startFrame() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	glDrawBuffer(attachments[deferredShadingColorTextureCount - 1]);
	glClear(GL_COLOR_BUFFER_BIT);
}

/*The pure geometry pass uses all attachments except the last one.*/
void GBuffer::bindForGeometryPass() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	glDrawBuffers(deferredShadingColorTextureCount - 1, attachments);
}

/*In the stencil test we are not writing to the color buffer, only the stencil buffer. Indeed, even our FS is empty. However, in that case the default output color from the FS is black. In order to avoid garbaging the final buffer with a black image of the bounding sphere we disable the draw buffers here.*/
void GBuffer::bindForStencilPass()
{
	glDrawBuffer(GL_NONE);
}

/*Binds the textures for usage in the shader to render into the frame buffer.*/
void GBuffer::bindForLightPass()
{
	glDrawBuffer(attachments[deferredShadingColorTextureCount-1]);

	for (int i = 0; i < deferredShadingColorTextureCount-1; i++){
		glActiveTexture(GL_TEXTURE0+i);
		// TODO glUniform1i(locationOfTextureinDeferredShader.frag, i);
		glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[i]);
	}
}

/* When we get to the final pass our final buffer is populated with the final image. Here we set things up for the blitting that takes place in the main application code. The default FBO is the target and the G Buffer FBO is the source.*/
void GBuffer::bindForFinalPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glReadBuffer(attachments[deferredShadingColorTextureCount - 1]);
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets and post-processing effects.
void GBuffer::renderQuad(){
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	for (int i = 0; i < deferredShadingColorTextureCount; i++){
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, i);
	}
}

// The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation)
float GBuffer::calcPointLightBSphere(LightNode* ln)
{
	float maxChannel = fmax(fmax(ln->light.diffuse.x, ln->light.diffuse.y), ln->light.diffuse.z);

	// TODO check if the last 1 is correct as the last number

	//float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear - 4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - 256 * MaxChannel * Light.DiffuseIntensity))) 
	// /
	//(2 * Light.Attenuation.Exp);

	return (-ln->light.shiConLinQua.z + sqrtf(ln->light.shiConLinQua.z * ln->light.shiConLinQua.z - 4 * ln->light.shiConLinQua.w * (ln->light.shiConLinQua.w - 256 * maxChannel * 1.0f)))
		/
		(2 * ln->light.shiConLinQua.w);
}


GBuffer::~GBuffer()
{
	glDeleteTextures(4, positionNormalColorHandles);

	glDeleteFramebuffers(1, &handle);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
}
