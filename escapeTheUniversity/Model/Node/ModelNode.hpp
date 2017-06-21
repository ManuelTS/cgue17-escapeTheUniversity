#pragma once

#include "Node.hpp"
#include "..\..\Shader.hpp"
#include "..\..\RenderLoop.hpp"
#include "..\Mesh\Mesh.hpp"
#include <vector>
#include "Bullet/btBulletDynamicsCommon.h"

/**This is the default node containing information about a model.*/
class ModelNode : public Node
{
public:
	vector<Mesh*> meshes; // Contains all meshes of this node.
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Position the node's geometrical center, in world coords relative to the parent.
	float radius = 0.0f; // Radius of the bounding sphere of this node, it is the biggest mesh value 

	vector<int>* indices; // Pointer to all sequential indices of all vertices in an array of this node, needed for example for btTriangleIndexVertexArray
	vector<float>* vertices; // Pointer to all sequential vertices in an array of this node, needed for example for btTriangleIndexVertexArray
	bool indicesVerticesArray = false; // Flag to generate the indices pointer and vertices only once. If true the pointers indices and vertices are set. Default: false

	glm::mat4 modelMatrix; // Object space MM of this node
	glm::mat4 inverseModelMatrix; // Inverse object space MM
	glm::mat4 hirachicalModelMatrix; // Ancestor multipled Model matrix, e.g. in position in world space
	glm::mat4 inverseHirachicalModelMatrix; // Ancestor calculated Model matrix
	btCollisionObject* collisionObject = nullptr; // The collision of this node, nullptr if none was generated
	btRigidBody* rigidBody = nullptr; // The rigid body of this node, nullptr if none was generated, since pointer casting does not work more pointers of bullet are needed, why having inheriance in the first place? -.-

	bool render = true; // True if this node should be rendered false if not, default: true
	bool stencil = false; // True if an extra stencil FBO for this node VAO should be generated in the mesh constructor, false if not. Default: false
	bool bounding = false; // True if an bounding VHACD should be generated, false if not. Default: false
	bool shadow = false; // True if shadow shaders are used, false if not. The GLSL locations are equal of the same, needed objects

	/*This is the default node containing information about a model.*/
	ModelNode();
	~ModelNode();

	void draw() override;
	vector<int>* getAllIndices(); // Returns a pointer to all vertex indices of all meshes in this node. Delete vector after usage!
	vector<float>* getAllVertices(); // Returns a pointer to all vertices of all meshes in this node. Delete vector after usage!
	void setModelMatrix(); // Sets the model matrix and computes the inverse one too, translates it with the position and calculates the world space position too
	glm::vec3 getWorldPosition(); // Returns the world coords of this node

	static const unsigned int modelLocation = 0; // gBuffer.vert
	static const unsigned int inverseModelLocation = 4; // gBuffer.vert
	static const unsigned int debugFlagLocation = 256; // gBuffer.frag
	static const unsigned int viewMatrixStencilLocation = 4; // gBuffer.frag
	static const unsigned int projectionMatrixStencilLocation = 8; // gBuffer.frag
private:
};