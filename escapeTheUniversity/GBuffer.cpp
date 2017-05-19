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
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[0], positionNormalColorHandles[0], 0);

	// World space coords and specular color in gBuffer.frag and deferredShading.frag
	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, attachments[1], positionNormalColorHandles[1], 0);

	glBindTexture(GL_TEXTURE_2D, positionNormalColorHandles[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, positionNormalColorHandles[2], 0);

	Debugger::getInstance()->checkWholeFramebufferCompleteness();

	glDrawBuffers(deferredShadingColorTextureCount, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glBindVertexArray(0);
}

/*Binds the textures for usage in the shader to render into the frame buffer.*/
void GBuffer::bindTextures()
{
	for (int i = 0; i < deferredShadingColorTextureCount; i++)
	{
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
		glBindTexture(GL_TEXTURE_2D, 0);
	}
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
}
