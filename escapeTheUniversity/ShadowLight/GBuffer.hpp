#pragma once

#include "..\Model\Node\LightNode.hpp"
#include "..\Shader.hpp"

/*Gbuffer used for deferred rendering in the renderLoop.*/
class GBuffer
{
private:
	unsigned int positionNormalColorHandles[4];

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
	Shader* stencilShader = new Shader("stencil"); // Set up schaders

	GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT);
	~GBuffer();

	void startFrame(); // At the start of each frame we need to clear the final texture which is attached to attachment point number 3
	void bindForGeometryPass(); // Previously the FBO in the G Buffer was static (in terms of its configuration) and was set up in advance so we just had to bind it for writing when the geometry pass started. Now we keep changing the FBO to we need to config the draw buffers for the attributes each time.
	void bind4LightPass(); // Binds the FBO as draw and sets the attachment 2 as target
	void bindTextures(); // Bindes the textures used
	float calcPointLightBSphere(LightNode* ln); /*Render the bounding sphere based on the light params.*/
	void renderQuad(); // Renders the stenciled quad
	void finalPass(const unsigned int width, const unsigned int height); // Sets the accumulating FBO to read and the standard FBO (screen) to draw and blits the first in the second FBO
};

