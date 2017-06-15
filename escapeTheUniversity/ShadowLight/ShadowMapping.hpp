#pragma once
#define GLM_FORCE_RADIANS // Use this for radiant calculation, the GLM one does not work!

#include "../Shader.hpp"
#include "../Model/Node/Node.hpp"
#include "../Model/Node/LightNode.hpp"

// Contains all information needed for the renderLoop.cpp to perform shadow mapping with PCF
class ShadowMapping
{
	private:
		const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024; // Size of the shadow (depth) map texture
		unsigned int depthFBO; // Handle of our shadow (depth) map
	public:
		Shader* shadowShader = new Shader("shadow");

		ShadowMapping();
		~ShadowMapping();

		void renderDepthMap(Node* root, LightNode* ln, const float farPlane, const float FOV, const unsigned int screenWidth, const unsigned int screenHeight); //  render depth of scene from light's perspective to texture (shadow, depth map)
		void draw(Node* current); // Draws the geometry from the lights view, but only their position is written into the shader and the resulting depth in the depth texture map than used for shadows
};
