#pragma once

#include "Node.hpp"
#include "..\..\Shader.hpp"
#include "..\..\RenderLoop.hpp"
#include "..\Mesh\Mesh.hpp"

/**This is the default node containing information about a model.*/
class ModelNode : public Node
{
public:
	std::vector<Mesh*> meshes; // Contains all meshes of this node.
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Position of this node, is any vertex of the node, standard value if unset is a zero vector.
	glm::vec3 pivot = glm::vec3(0.0f, 0.0f, 0.0f); // Pivot of this node, standard value if unset is a zero vector.
	
	glm::mat4 modelMatrix; // MM of this node
	glm::mat4 inverseModelMatrix; // Inverse MM

	/*This is the default node containing information about a model.*/
	ModelNode();
	~ModelNode();

	void draw() override;
	bool isEmpty(); // Returns true if empty, false if not

protected:
	const GLint modelLocation = 0; // gBuffer.vert
	const GLint inverseModelLocation = 4; // gBuffer.vert
private:
};