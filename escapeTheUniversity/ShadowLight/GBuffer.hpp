#pragma once

#include "..\Model\Node\LightNode.hpp"
#include "..\Shader.hpp"

/*Gbuffer used for deferred rendering in the renderLoop.*/
class GBuffer
{
private:
	unsigned int positionNormalColorHandles[3];

	unsigned int quadVAO;
	unsigned int quadVBO;
	const int quadPositionLocation = 0; // Used in deferredShading.vert
	const int quadTextureCoordsLocation = 1; // Used in deferredShading.vert
	const int colorAndNormalTexLocation = 1; // Used in deferredShading.frag
	const int positionAndShininessTexLocation = 2; // Used in deferredShading.frag

	const int deferredShadingColorTextureCount = 2; // Texture count used in this gBuffer
public:
	unsigned int handle;

	Shader* gBufferShader = new Shader("gBuffer"); // Set up schaders
	Shader* deferredShader = new Shader("deferredShading");

	GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT);
	~GBuffer();

	void bindTextures(); // Bindes the textures used
	float calcPointLightBSphere(LightNode* ln); /*Render the bounding sphere based on the light params.*/
	void renderQuad();
};
