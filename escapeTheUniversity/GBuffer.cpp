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
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F,	MAX_WIDTH, MAX_HEIGHT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, positionNormalColorHandles[3], 0);
	

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
	glDrawBuffers(2, attachments);
}

/*Binds the textures for usage in the shader to render into the frame buffer.*/
void GBuffer::bindForLightPass()
{
	glDrawBuffer(attachments[2]);

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
	glDrawBuffer(attachments[2]);
	glClear(GL_COLOR_BUFFER_BIT);
}

/* When we get to the final pass our final buffer is populated with the final image. Here we set things up for the blitting that takes place in the main application code. The default FBO is the target and the G Buffer FBO is the source.*/
void GBuffer::bindForFinalPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glReadBuffer(attachments[2]);
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

// The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation). It returns the effecting max light distance which is the radius of the light sphere.
float GBuffer::calcPointLightBSphere(LightNode* ln)
{
	glm::vec3 lightLuminance = glm::cross(glm::vec3(ln->light.diffuse), glm::vec3(0.2126, 0.7152, 0.0722));// Get light's luminance using Rec 709 luminance formula
	const float maxLuminance = 0.01 / fmax(fmax(lightLuminance.x, lightLuminance.y), lightLuminance.z); // min luminance divided by max luminance, from https://gamedev.stackexchange.com/questions/51291/deferred-rendering-and-point-light-radius
	const float maxChannel = fmax(fmax(ln->light.diffuse.x, ln->light.diffuse.y), ln->light.diffuse.z);

	// Calculation on: http://ogldev.atspace.co.uk/www/tutorial36/tutorial36.html
	return (float) (-ln->light.shiConLinQua.z + sqrtf(ln->light.shiConLinQua.z * ln->light.shiConLinQua.z - 4 * ln->light.shiConLinQua.w * (ln->light.shiConLinQua.w - 256.0f * maxChannel * maxLuminance)))
		/
		(2.0f * ln->light.shiConLinQua.w);
}


GBuffer::~GBuffer()
{
	glDeleteTextures(4, positionNormalColorHandles);

	glDeleteFramebuffers(1, &handle);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
}
