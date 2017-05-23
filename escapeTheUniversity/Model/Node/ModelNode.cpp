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

void ModelNode::setModelMatrix(glm::mat4* m)
{
	modelMatrix = glm::translate(*m, position);
	inverseModelMatrix = glm::inverseTranspose(modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
	hirachicalModelMatrix = getModelMatrix();
	inverseHirachicalModelMatrix = glm::inverseTranspose(hirachicalModelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
}

glm::mat4 ModelNode::getModelMatrix() {
	glm::mat4 calculated = modelMatrix;

	if(parent)
	{
		ModelNode* p = dynamic_cast<ModelNode*>(parent);

		if (p)
			calculated *= p->getModelMatrix();
	}

	return calculated;
}

/*Draws all the involved meshes by looping over them and call their draw functions.*/
void ModelNode::draw()
{
	unsigned int size = meshes.size();

	if (size > 0)
	{

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(hirachicalModelMatrix)); // Only use the hirachical MMs here? TODO
		glUniformMatrix4fv(inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseHirachicalModelMatrix));

		for (unsigned int i = 0; i < size; i++)
			meshes[i]->draw();
	}
}
