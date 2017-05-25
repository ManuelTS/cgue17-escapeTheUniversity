#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "TransformationNode.hpp"
#include "ModelNode.hpp"
#include "..\..\RenderLoop.hpp"


TransformationNode::TransformationNode()
{
}


TransformationNode::~TransformationNode()
{
}

void TransformationNode::switchState()
{
	transform = !transform;
}

float TransformationNode::change(bool plus)
{
	float newRadiant = RADIANT * RenderLoop::getInstance()->deltaTime;
	
	if (!plus)
		newRadiant = -newRadiant;

	for (Node* child : children)
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(child);

		if (mn)
		{
			mn->modelMatrix = glm::translate(mn->modelMatrix, -mn->position); // Translate to origin to rotate there
			mn->modelMatrix = glm::rotate(mn->modelMatrix, newRadiant, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it only on y axis
			mn->modelMatrix = glm::translate(mn->modelMatrix, mn->position); // Translate rotated matrix bock to position
			mn->inverseModelMatrix = glm::inverseTranspose(mn->modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
			// TODO check what to use here with the hirachical ones
		}
	}

	return newRadiant;
}

void TransformationNode::draw()
{
	if (transform && currentRotationRadiant < MAX_ROTATION_RADIANT)
		currentRotationRadiant += change(true);
	else if (!transform && currentRotationRadiant > 0)
		currentRotationRadiant += change(false);
}