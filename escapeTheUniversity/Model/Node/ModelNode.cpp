#include <GLM\gtc\type_ptr.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "ModelNode.hpp"

ModelNode::ModelNode()
{
	//modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	//modelMatrix = glm::rotate(modelMatrix,0.0f, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it
	//modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
	inverseModelMatrix = glm::inverseTranspose(modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
}
ModelNode::~ModelNode(){}

void ModelNode::setModelMatrix()
{
	modelMatrix = glm::translate(glm::mat4(), position);
	inverseModelMatrix = glm::inverseTranspose(modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
	hirachicalModelMatrix = glm::translate(glm::mat4(), getWorldPosition()); // Don't use the MM here only *m!
	inverseHirachicalModelMatrix = glm::inverseTranspose(hirachicalModelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
}

glm::vec3 ModelNode::getWorldPosition() {
	glm::vec3 calculated = position;

	if(parent)
	{
		ModelNode* p = dynamic_cast<ModelNode*>(parent);

		if (p)
			calculated += p->getWorldPosition();
	}

	return calculated;
}

/*Draws all the involved meshes by looping over them and call their draw functions.*/
void ModelNode::draw()
{
	unsigned int size = meshes.size();

	if (size > 0)
	{
		if(bounding)
		{
			btTransform trans;
			rigitBody->getMotionState()->getWorldTransform(trans);
			glm::mat4 temp = hirachicalModelMatrix;
			trans.getOpenGLMatrix(glm::value_ptr(hirachicalModelMatrix));

			if(temp != hirachicalModelMatrix)
				inverseHirachicalModelMatrix = glm::inverseTranspose(inverseHirachicalModelMatrix);
			
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(hirachicalModelMatrix));
			glUniformMatrix4fv(inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseHirachicalModelMatrix));
		}
		else {
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(hirachicalModelMatrix));
			glUniformMatrix4fv(inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseHirachicalModelMatrix));
		}

		for (unsigned int i = 0; i < size; i++)
			meshes[i]->draw();
	}
}

vector<int>* ModelNode::getAllIndices()
{
	unsigned int size = meshes.size();

	if (size > 0)
	{
		vector<int>* allIndices = new vector<int>(); // Needs to be deleted else where
		
		for (unsigned int i = 0, vertexOffset = 0; i < size; i++) // Go through all meshes
		{
			vertexOffset = allIndices->size();// The indices for different meshes start all at one but are needed to be continous (from the last mesh only !) since all vertices read are continous too

			for (unsigned int j = 0; j < meshes[i]->indices.size(); j++) // Go through all mesh vertices
				allIndices->push_back(vertexOffset + meshes[i]->indices[j]); // cast unsigned int to int because of vhacd method signature
		}

		return allIndices;
	}

	return nullptr;
}

vector<float>* ModelNode::getAllVertices() {
	unsigned int size = meshes.size();

	if (size > 0)
	{
		vector<float>* allVertices = new vector<float>();

		for (unsigned int i = 0; i < size; i++) // Go through all meshes
			for (unsigned int j = 0; j < meshes[i]->data.size(); j++) // Go through all mesh vertices
			{
				const glm::vec3 position = meshes[i]->data[j].position; // Get all the vertices
				allVertices->push_back(position.x);
				allVertices->push_back(position.y);
				allVertices->push_back(position.z);
			}

		return allVertices;
	}

	return nullptr;
}