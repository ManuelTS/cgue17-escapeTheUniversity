#include <GLM\gtc\type_ptr.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "ModelNode.hpp"

ModelNode::ModelNode()
{
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	//modelMatrix = glm::rotate(modelMatrix,0.0f, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it
	//modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
	inverseModelMatrix = glm::inverseTranspose(modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
}
ModelNode::~ModelNode(){}

/*Draws all the involved meshes by looping over them and call their draw functions.*/
void ModelNode::draw()
{
	unsigned int size = meshes.size();

	if (size > 0)
	{

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseModelMatrix));

		for (unsigned int i = 0; i < size; i++)
			meshes[i]->draw();
	}
}

void ModelNode::setModelMatrix(glm::mat4* m) 
{
	modelMatrix = *m;
	inverseModelMatrix = glm::inverseTranspose(*m); // Transpose and inverse on the CPU because it is very costly on the GPU
}

bool ModelNode::isEmpty() {
	if (position.x == 0.0f && position.y == 0.0f && position.z == 0.0f && pivot.x == 0.0f && pivot.y == 0.0f && pivot.z == 0.0f && isLeaf())
		return true;
	return false;
}

glm::vec3 ModelNode::getLocalWorldPosition()
{
	// TODO https://gamedev.stackexchange.com/questions/108945/assimp-transformation-hierarchy-and-animations
	// Yes you need to multiply the child node's transform with the parent node's transform to obtain the final world transform for the mesh at that node. You basically have to walk down the hierarchy to the child nodes, multiplying all the transform together to get the transform for each child node's mesh.
	glm::vec3 calculated = position;

	if (glm::all(glm::equal(calculated, glm::vec3(0.0f))))// Check if not a null vector
		calculated = glm::vec3(1.0f); // Does not change multiplication on other recursions
	
	if (parent != nullptr) 
	{
		ModelNode* mParent = dynamic_cast<ModelNode*>(parent);

		if (mParent != nullptr)
			calculated += mParent->getLocalWorldPosition();
	}

	return calculated;
}