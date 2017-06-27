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
	const float AMBIENT_LIGHT = 0.1f;
	struct Light{ // Due to openGL byte ordering only vec4 usage
		glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // xyz = world coord position of light, w = light sphere radius
		glm::vec4 diffuse = glm::vec4(0.0f, 0.0f, 0.0f, AMBIENT_LIGHT); // rgb = rgb of diffuse light, a = ambient light coefficient to decrase the diffuse light and use it as ambient one
		// The diffuse light colors are the same as the specular light colors, in blender specular light colors indefinable
		glm::vec4 shiConLinQua = glm::vec4(32.0f, 1.0f, 0.22f, 0.20f); // shininess, constant, linear, quadratic http://www.ogre3d.org/tikiwiki/tiki-index.php?page=-Point+Light+Attenuation
	} light;

	const unsigned int UBO;
	const unsigned int arrayIndex;

	LightNode(const unsigned int _UBO, const unsigned int _arrayIndex);
	~LightNode();
private:
};

