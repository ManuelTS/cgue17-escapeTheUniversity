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
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Position of this node, is any vertex of the node, standard value if unset is a zero vector. In object space
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
	void setModelMatrix(glm::mat4* m); // Sets the model matrix and computes the inverse one too, translates it with the position and calculates the world space position too
	glm::mat4 getModelMatrix(); // Returns the hirachical model matrix, e.g. world coords

protected:
	const GLint modelLocation = 0; // gBuffer.vert
	const GLint inverseModelLocation = 4; // gBuffer.vert
private:
};