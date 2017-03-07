#pragma once

/*Gbuffer used for deferred shading.*/
class GBuffer
{
private:
	unsigned int positionNormalColorHandles[3];

	unsigned int quadVAO;
	unsigned int quadVBO;
	const int deferredShadingColorTextureCount = 2;
	const int quadPositionLocation = 0; // Used in deferredShading.vert
	const int quadTextureCoordsLocation = 1; // Used in deferredShading.vert
	const int colorAndNormalTexLocation = 1; // Used in deferredShading.frag
	const int positionAndShininessTexLocation = 2; // Used in deferredShading.frag
public:
	unsigned int handle;

	GBuffer(const int MAX_WIDTH, const int MAX_HEIGHT);
	~GBuffer();

	void GBuffer::bindTextures();
	void GBuffer::renderQuad();
};

