#pragma once

#include "..\Model\Node\LightNode.hpp"
#include "..\Shader.hpp"

/*Gbuffer used for deferred rendering in the renderLoop.*/
class GBuffer
{
private:
	unsigned int positionNormalColorHandles[4];

	unsigned int quadVAO; // Is the VAO of the quad drawn for ambient light (could also be ambient)
	unsigned int quadVBO;
	unsigned int quadEBO;// Create an element array
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

	void clearFrame();
	void bindForGeometryPass(); // Previously the FBO in the G Buffer was static (in terms of its configuration) and was set up in advance so we just had to bind it for writing when the geometry pass started. Now we keep changing the FBO to we need to config the draw buffers for the attributes each time.
	void bind4StencilPass(); // Bind the FBO and disable color rendering
	void bind4LightPass(); // Binds the FBO as draw and sets the attachment 2 as target
	void bindTextures(); // Bindes the textures used
	float calcPointLightBSphere(LightNode* ln); /*Render the bounding sphere based on the light params.*/
	void unbindTexture(); // Ubindes the textures used in the gbuffer
	void drawDirectionalLight(); // directional light pass, it light does not need a stencil test because its volume is unlimited and the final pass simply copies the texture, in our case this is obly the ambient light
	void finalPass(const unsigned int width, const unsigned int height); // Sets the accumulating FBO to read and the standard FBO (screen) to draw and blits the first in the second FBO
};

