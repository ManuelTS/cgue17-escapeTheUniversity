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
	modelMatrix = *m;
	inverseModelMatrix = glm::inverseTranspose(*m); // Transpose and inverse on the CPU because it is very costly on the GPU
}

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

int* ModelNode::getAllIndices() {
	unsigned int size = meshes.size();

	if (size > 0)
	{
		vector<int> allIndices;

		for (unsigned int i = 0; i < size; i++) // Go through all meshes
			for (unsigned int j = 0; j < meshes[i]->indices.size(); j++) // Go through all mesh vertices
			{
				unsigned int uint = meshes[i]->indices[j];
				int castInt = uint;
				int cast2Int = static_cast<int>(uint);
				allIndices.push_back(meshes[i]->indices[j]); // cast unsigned int to int because of vhacd method signature
			}

		return allIndices.data();
	}

	return nullptr;
}
