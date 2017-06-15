#include "ShadowMapping.hpp"
#include "../Model/Node/ModelNode.hpp"
#include "../Camera/Frustum.hpp"
#include <GLM\gtc\type_ptr.hpp>

ShadowMapping::ShadowMapping()
{
	// create depth texture
	glGenTextures(1, &dephMapTextureHandle);
	glBindTexture(GL_TEXTURE_2D, dephMapTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 }; // Avoid shadow map over sampling
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dephMapTextureHandle, 0);
	glDrawBuffer(GL_NONE); // No color buffers are needed
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMapping::~ShadowMapping()
{
	delete shadowShader;
}

void ShadowMapping::renderInDepthMap(Node* root, LightNode* ln, float farPlane, const float FOV, const unsigned int screenWidth, const unsigned int screenHeight)
{
	const float near_plane = 1.0f;
	farPlane = 7.5f;
	Frustum* frustum = Frustum::getInstance();
	// Calculate light space matirx
	glm::mat4 lightProjection = glm::perspective(frustum->degreesToRadians(FOV), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, farPlane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, farPlane);
	glm::vec3 lightPosition = glm::vec3(ln->light.position);
	glm::vec3 lightFront = glm::vec3(0.0f);
	glm::vec3 lightUp = glm::vec3(0.0, 1.0, 0.0);
	glm::mat4 lightView = glm::lookAt(lightPosition, lightFront, lightUp);
	lightSpaceMatrix = lightProjection * lightView; // Combining these two gives us a light space transformation matrix that transforms each world-space vector into the space as visible from the light source; exactly what we need to render the depth map.

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	shadowShader->useProgram();
	glUniformMatrix4fv(shadowShader->SHADOW_LIGHT_SPACE_MATRIX_LOCATION, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	//frustum->setCamDef(lightPosition, lightFront, lightUp);
	//frustum->setCamInternals(degreesFOV, SHADOW_WIDTH, SHADOW_HEIGHT);

	draw(root);

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// reset viewport
	glViewport(0, 0, screenWidth, screenHeight);

	//Frustum* frustum = Frustum::getInstance();
	//frustum->setCamDef(position, front, up);
	//frustum->setCamInternals(FOV, width, height);
}

void ShadowMapping::draw(Node* current)
{
	ModelNode* mn = dynamic_cast<ModelNode*>(current);

	if (mn)// TODO: frustum culling
	{
		mn->shadow = true;
		mn->draw();
		mn->shadow = false;
	}

	for (Node* child : current->children)
		draw(child);
}

void ShadowMapping::bindTexture()
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	// TODO glUniform1i(locationOfTextureinDeferredShader.frag, i);
	glBindTexture(GL_TEXTURE_2D, dephMapTextureHandle);
}

void ShadowMapping::unbindTexture()
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}