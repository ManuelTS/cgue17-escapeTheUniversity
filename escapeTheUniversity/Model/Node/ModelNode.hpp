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
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Position the node's center, in world coords relative to the parent.
	float radius = 0.0f; // Radius of the bounding sphere of this node, it is the biggest mesh value 
	glm::vec3 pivot = glm::vec3(0.0f, 0.0f, 0.0f); // Pivot of this node, standard value if unset is a zero vector.
	
	glm::mat4 modelMatrix; // Object space MM of this node
	glm::mat4 inverseModelMatrix; // Inverse object space MM
	glm::mat4 hirachicalModelMatrix; // Ancestor multipled Model matrix, e.g. in position in world space
	glm::mat4 inverseHirachicalModelMatrix; // Ancestor calculated Model matrix

	bool render = true; // True if this node should be rendered false if not, default: true
	bool stencil = false; // True if an extra stencil FBO for this node VAO should be generated in the mesh constructor, false if not. Default: false

	/*This is the default node containing information about a model.*/
	ModelNode();
	~ModelNode();

	void draw() override;
	vector<int>* getAllIndices(); // Returns a pointer to all vertex indices of all meshes in this node. Delete vector after usage!
	vector<float>* getAllVertices(); // Returns a pointer to all vertices of all meshes in this node. Delete vector after usage!
	void setModelMatrix(); // Sets the model matrix and computes the inverse one too, translates it with the position and calculates the world space position too
	glm::vec3 getWorldPosition(); // Returns the world coords of this node

protected:
	const GLint modelLocation = 0; // gBuffer.vert
	const GLint inverseModelLocation = 4; // gBuffer.vert
private:
};