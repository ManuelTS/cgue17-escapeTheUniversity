#include <GLM\gtc\type_ptr.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "ModelNode.hpp"
#include "TransformationNode.hpp"

ModelNode::ModelNode()
{
	//modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	//modelMatrix = glm::rotate(modelMatrix,0.0f, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it
	//modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
	inverseModelMatrix = glm::inverseTranspose(modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
}
ModelNode::~ModelNode()
{
	if (indicesVerticesArray && indices != nullptr) {
		indices->clear();
		delete indices;
	}
	if (indicesVerticesArray && vertices != nullptr) {
		vertices->clear();
		delete vertices;
	}
}

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
		if(bounding && rigidBody)
		{
			btTransform trans;
			rigidBody->getMotionState()->getWorldTransform(trans);
			glm::mat4 temp = hirachicalModelMatrix;
			trans.getOpenGLMatrix(glm::value_ptr(hirachicalModelMatrix)); // Get transformed position from bullet

			if(temp != hirachicalModelMatrix)
			{
				position = (temp - hirachicalModelMatrix)[3]; // Get changed position from bullet in object space
				modelMatrix[3] = glm::vec4(position, 1.0f);
				inverseHirachicalModelMatrix = glm::inverseTranspose(inverseHirachicalModelMatrix);
			}
			
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(hirachicalModelMatrix));
			glUniformMatrix4fv(inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseHirachicalModelMatrix));
		}
		else
		{
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
		indices = new vector<int>(); // Needs to be deleted else where
		
		for (unsigned int i = 0, vertexOffset = 0; i < size; i++) // Go through all meshes
		{
			vertexOffset = indices->size();// The indices for different meshes start all at one but are needed to be continous (from the last mesh only !) since all vertices read are continous too

			vector<int>* temp = meshes[i]->getAllIndices();

			for (unsigned int j = 0; j < temp->size(); j++) // Go through all mesh vertices
				indices->push_back(vertexOffset + temp->at(j)); // cast unsigned int to int because of vhacd method signature
		}

		indicesVerticesArray = true;
		return indices;
	}

	return nullptr;
}

vector<float>* ModelNode::getAllVertices() {
	unsigned int size = meshes.size();

	if (size > 0)
	{
		vertices = new vector<float>();

		for (unsigned int i = 0; i < size; i++) // Go through all meshes
		{
			Mesh* m = meshes.at(i);
			vector<float>* temp = m->getAllVertices();

			vertices->insert(vertices->end(), temp->begin(), temp->end());
			
			delete temp;
		}

		indicesVerticesArray = true;
		return vertices;
	}

	return nullptr;
}