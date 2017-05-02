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

float TransformationNode::change(bool plus) {
	double deltaTime = 1.0f / RenderLoop::getInstance()->deltaTime;
	float newAngle = glm::radians(ANGLE*deltaTime);
	
	if (!plus)
		newAngle = -newAngle;

	for (Node* child : children)
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(child);

		if (mn != nullptr)
		{
			mn->modelMatrix = glm::translate(mn->modelMatrix, -mn->position);
			mn->modelMatrix = glm::rotate(mn->modelMatrix, newAngle, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it
			mn->modelMatrix = glm::translate(mn->modelMatrix, mn->position);
			mn->inverseModelMatrix = glm::inverseTranspose(mn->modelMatrix); // Transpose and inverse on the CPU because it is very costly on the GPU
		}
	}

	return newAngle;
}

void TransformationNode::draw()
{
	if (transform &&  currentRotation < MAX_ROTATION)
		currentRotation += change(true);
	else if (!transform && currentRotation > 0)
		currentRotation += change(false);
}