#pragma once
#include "Model\Node\LightNode.hpp"

/*Gbuffer used for deferred shading.*/
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

	const int deferredShadingColorTextureCount = 3; // Texture count used in this gBuffer
public:
	unsigned int handle;
	const int viewLocation = 4; // Used in deferredShadingStencil.vert
	const int projectionLocation = 8; // Used in deferredShadingStencil.vert

	GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT);
	~GBuffer();

	void startFrame(); /*At the start of each frame we need to clear the final texture which is attached to attachment point number 2.*/
	void bindForGeometryPass(); /*The pure geometry pass uses all attachments except the last one.*/
	void bindForLightPass();
	void bindForFinalPass(); /* When we get to the final pass our final buffer is populated with the final image. Here we set things up for the blitting that takes place in the main application code. The default FBO is the target and the G Buffer FBO is the source.*/
	float calcPointLightBSphere(LightNode* ln); /*Render the bounding sphere based on the light params.*/
	void renderQuad();
};

