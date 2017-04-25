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
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[0], GL_TEXTURE_2D, positionNormalColorHandles[0], 0);

	// World space coords and specular color in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[1], GL_TEXTURE_2D, positionNormalColorHandles[1], 0);

	/* Final, the one rendered first and blitted. This is a good place to discuss why we added an intermediate color buffer in 
	the G Buffer instead of rendering directly to the screen. The thing is that our G Buffer combines as a target the buffers
	for the attributes with the depth/stencil buffer. When we run the point light pass we setup the stencil stuff and we need
	to use the values from the depth buffer. Here we have a problem - if we render into the default FBO we won't have access
	to the depth buffer from the G Buffer. But the G Buffer must have its own depth buffer because when we render into its 
	FBO we don't have access to the depth buffer from the default FBO. Therefore, the solution is to add to the G Buffer FBO 
	a color buffer to render into and in the final pass blit it to the default FBO color buffer.*/
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[2], GL_TEXTURE_2D, positionNormalColorHandles[2], 0);

	// Depth and stencil buffer
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[3]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH32F_STENCIL8,	MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, positionNormalColorHandles[3], 0);
	

	Debugger::getInstance()->checkWholeFramebufferCompleteness();

	glDrawBuffers(deferredShadingColorTextureCount, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glBindVertexArray(0);
}

/*The pure geometry pass uses all attachments except the last one.*/
void GBuffer::bindForGeometryPass() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	glDrawBuffers(deferredShadingColorTextureCount - 2, attachments);
}

/*Binds the textures for usage in the shader to render into the frame buffer.*/
void GBuffer::bindForLightPass()
{
	glDrawBuffer(attachments[deferredShadingColorTextureCount - 1]);

	for (int i = 0; i < deferredShadingColorTextureCount - 1; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		// TODO glUniform1i(locationOfTextureinDeferredShader.frag, i);
		glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[i]);
	}
}

/*At the start of each frame we need to clear the final texture which is attached to attachment point number 2.*/
void GBuffer::startFrame() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	glDrawBuffer(attachments[deferredShadingColorTextureCount - 1]);
	glClear(GL_COLOR_BUFFER_BIT);
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

// The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation). It returns the effecting max light distance.
float GBuffer::calcPointLightBSphere(LightNode* ln)
{
	float maxChannel = fmax(fmax(ln->light.diffuse.x, ln->light.diffuse.y), ln->light.diffuse.z);

	// Calculation on: http://ogldev.atspace.co.uk/www/tutorial36/tutorial36.html
	return (-ln->light.shiConLinQua.z + sqrtf(ln->light.shiConLinQua.z * ln->light.shiConLinQua.z - 4 * ln->light.shiConLinQua.w * (ln->light.shiConLinQua.y - 256 * maxChannel * ln->light.shiConLinQua.x)))
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
