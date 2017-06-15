#pragma once
#define GLM_FORCE_RADIANS // Use this for radiant calculation, the GLM one does not work!

#include "../Shader.hpp"
#include "../Model/Node/Node.hpp"
#include "../Model/Node/LightNode.hpp"
#include <GLM\glm.hpp>

// Contains all information needed for the renderLoop.cpp to perform shadow mapping with PCF
class ShadowMapping
{
private:
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024; // Size of the shadow (depth) map texture
	unsigned int depthFBO; // Handle of our shadow (depth) map
	unsigned int dephMapTextureHandle; // texture unit handle of the shadow (depth) map
	unsigned int textureUnit = 2; // for the depth map, deferred shading uses already texture unit number 0 and 1
	
public:
	const unsigned int SHADOW_LIGHT_SPACE_MATRIX_LOCATION = 1; // in the deferredShading.frag
	Shader* shadowShader = new Shader("shadow");
	glm::mat4 lightSpaceMatrix;  // Combining these two gives us a light space transformation matrix that transforms each world-space vector into the space as visible from the light source; exactly what we need to render the depth map.

	ShadowMapping();
	~ShadowMapping();

	void renderInDepthMap(Node* root, LightNode* ln, float farPlane, const float FOV, const unsigned int screenWidth, const unsigned int screenHeight); //  render depth of scene from light's perspective to texture (shadow, depth map)
	void draw(Node* current); // Draws the geometry from the lights view, but only their position is written into the shader and the resulting depth in the depth texture map than used for shadows
	void bindTexture(); // Binds the texture unit in the deferredShader.frag to have the depthMap
	void unbindTexture(); // Unbindes the textures bound in bindTexture
};