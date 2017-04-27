#include "LightNode.hpp"

using namespace std;

LightNode::LightNode(const unsigned int _UBO, const unsigned int _arrayIndex) : UBO(_UBO), arrayIndex(_arrayIndex)
{}

LightNode::~LightNode()
{}

// For debugging, set ambient, diffuse, and specular to the argument
void LightNode::setAllLightComponents(glm::vec3 ads) {
	light.ambient = glm::vec4(ads,0.0f); // Great VS and c++ strike again on vec4 in method signature: Severity	Code	Description	Project	File	Line	Suppression State	Error	C2719	'ads': formal parameter with requested alignment of 16 won't be aligned	escapeTheUniversity	c:\users\goodone\documents\visual studio 2015\projects\escapetheuniversity\escapetheuniversity\model\node\lightnode.hpp	28
	light.diffuse = glm::vec4(ads, 0.0f);
	light.specular = glm::vec4(ads, 0.0f);
}