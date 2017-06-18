#pragma once
#include "Node.hpp"
#include "..\..\Shader.hpp"
#include <GLM\gtc\type_ptr.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>

class LightNode : public Node
{
public:
	/*Defines the light used in the shader, the position vector has a swith in the fourth position, vec4.w = 0.0 direction vector for directional light, = 1.0 position vector for a point light. If in the shader 
	are only vec3 this is okay due to unit memory allocation.*/
	struct Light{ // Due to openGL byte ordering only vec4 usage
		glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // xyz = world coord position of light, w = flag if it should be drawn or not 1.0 = true = yes, otherwise false = no.
		glm::vec4 diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.01f); // rgb = rgb of diffuse light, a = ambient light coefficient to decrase the diffuse light and use it as ambient one
		glm::vec4 specular = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f); // rgb = rgb of diffuse light, a = light Sphere radius calculated in gBuffer#calcPointLightBSphere
		glm::vec4 shiConLinQua = glm::vec4(64.0f, 1.0f, 0.22f, 0.20f); // shininess, constant, linear, quadratic http://www.ogre3d.org/tikiwiki/tiki-index.php?page=-Point+Light+Attenuation
	} light;

	const unsigned int UBO;
	const unsigned int arrayIndex;

	LightNode(const unsigned int _UBO, const unsigned int _arrayIndex);
	~LightNode();
private:
};

