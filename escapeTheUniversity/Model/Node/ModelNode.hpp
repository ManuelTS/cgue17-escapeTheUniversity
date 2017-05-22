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

	bool render = true; // True if this node should be rendered false if not, default: true
	bool stencil = false; // True if an extra stencil FBO for this node VAO should be generated in the mesh constructor, false if not. Default: false

	/*This is the default node containing information about a model.*/
	ModelNode();
	~ModelNode();

	void draw() override;
	void setModelMatrix(glm::mat4* m); // Sets the model matrix and computes the inverse one too
	int* getAllIndices(); // Returns a pointer to all vertex indices of all meshes in this node
	float* getAllVertices(); // Returns a pointer to all vertices of all meshes in this node

protected:
	const GLint modelLocation = 0; // gBuffer.vert
	const GLint inverseModelLocation = 4; // gBuffer.vert
private:
};