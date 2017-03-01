#include <GL/glew.h>
#include "GBuffer.hpp"
#include "Initialization.hpp"
#include "./Debug/Debugger.hpp"

/*Gbuffer used for deferred shading.*/
GBuffer::GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	glGenTextures(3, positionNormalColorHandles);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

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

	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F,	MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,positionNormalColorHandles[2], 0);

	Debugger::getInstance()->checkWholeFramebufferCompleteness();

	glDrawBuffers(deferredShadingColorTextureCount, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glBindVertexArray(0);
}

/*Binds the textures for usage in the shader.*/
void GBuffer::bindTextures(){
	for (int i = 0; i < deferredShadingColorTextureCount; i++){
		glActiveTexture(GL_TEXTURE0+i);
		// TODO glUniform1i(locationOfTextureinDeferredShader.frag, i);
		glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[i]);
	}
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


GBuffer::~GBuffer()
{
	glDeleteTextures(3, positionNormalColorHandles);

	glDeleteFramebuffers(1, &handle);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
}
