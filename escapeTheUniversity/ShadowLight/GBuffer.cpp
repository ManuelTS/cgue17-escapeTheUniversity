#include <GL/glew.h>
#include "GBuffer.hpp"
#include "../Initialization.hpp"
#include "../Debug/Debugger.hpp"

/*Gbuffer used for deferred shading.*/
GBuffer::GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	glGenTextures(4, positionNormalColorHandles);
	const unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	// Normals, albedo color, and material index in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[0], positionNormalColorHandles[0], 0);

	// World space coords and specular color in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[1], positionNormalColorHandles[1], 0);

	// The thing is that our G Buffer combines as a target the buffers for the attributes with the depth/stencil buffer. When we run the point light pass we setup the stencil stuff and we need to use the values from the depth buffer. Here we have a problem - if we render into the default FBO we won't have access to the depth buffer from the G Buffer. But the G Buffer must have its own depth buffer because when we render into its FBO we don't have access to the depth buffer from the default FBO. 
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[2], positionNormalColorHandles[2], 0);

	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[3]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH32F_STENCIL8, MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, positionNormalColorHandles[3], 0);

	Debugger::getInstance()->checkWholeFramebufferCompleteness();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glBindVertexArray(0);
}

void GBuffer::bindForGeometryPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
	const unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear everything inside the buffer for new clean, fresh iteration
}

void GBuffer::bind4LightPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
}

/*Binds the textures for usage in the shader to render into the frame buffer.*/
void GBuffer::bindTextures()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT2); // attach auxiliary color buffer
	glClearColor(0, 0, 0, 1); // set fallback color for non touched fragments
	glClear(GL_COLOR_BUFFER_BIT); // set buffer color

	for (int i = 0; i < deferredShadingColorTextureCount; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		// TODO glUniform1i(locationOfTextureinDeferredShader.frag, i);
		glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[i]);
	}
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets and post-processing effects.
void GBuffer::unbindTexture()
{
	for (int i = 0; i < deferredShadingColorTextureCount; i++){
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void GBuffer::finalPass(const unsigned int width, const unsigned int height)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Sets the accumulating FBO to read and the standard FBO (screen) to draw and blits the first in the second FBO
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR); // Blit FBO to screen
}

// The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation). It returns the effecting max light distance which is the radius of the light sphere.
float GBuffer::calcPointLightBSphere(LightNode* ln)
{
	glm::vec3 lightLuminance = glm::cross(glm::vec3(ln->light.diffuse), glm::vec3(0.2126, 0.7152, 0.0722));// Get light's luminance using Rec 709 luminance formula
	const float maxLuminance = 0.02 / fmax(fmax(lightLuminance.x, lightLuminance.y), lightLuminance.z); // min luminance threshold divided by max luminance, from https://gamedev.stackexchange.com/questions/51291/deferred-rendering-and-point-light-radius
	const float maxChannel = fmax(fmax(ln->light.diffuse.x, ln->light.diffuse.y), ln->light.diffuse.z);

	// Calculation on: http://ogldev.atspace.co.uk/www/tutorial36/tutorial36.html
	return (float) (-ln->light.shiConLinQua.z + sqrtf(ln->light.shiConLinQua.z * ln->light.shiConLinQua.z - 4 * ln->light.shiConLinQua.w * (ln->light.shiConLinQua.w - 256.0f * maxChannel * maxLuminance)))
		/
		(2.0f * ln->light.shiConLinQua.w);
}

GBuffer::~GBuffer()
{
	glDeleteTextures(3, positionNormalColorHandles);

	glDeleteFramebuffers(1, &handle);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);

	delete gBufferShader;
	delete deferredShader;
}
