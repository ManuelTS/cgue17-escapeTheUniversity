#pragma once
#include "TransformationNode.hpp"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
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
	float newRadiant = RADIANT * RenderLoop::getInstance()->getTimeDelta();
	
	if (!plus)
		newRadiant = -newRadiant;

	for (Node* child : children)
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(child);

		if (mn)
		{
			mn->hirachicalModelMatrix = glm::translate(mn->hirachicalModelMatrix, -mn->position); // Translate to origin to rotate there
			mn->hirachicalModelMatrix = glm::rotate(mn->hirachicalModelMatrix, newRadiant, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it only on y axis
			mn->hirachicalModelMatrix = glm::translate(mn->hirachicalModelMatrix, mn->position); // Translate rotated matrix bock to position
			// - rotate is faster than inverseTranspose
			mn->inverseHirachicalModelMatrix = glm::translate(mn->inverseHirachicalModelMatrix, -mn->position); // Translate to origin to rotate there
			mn->inverseHirachicalModelMatrix = glm::rotate(mn->inverseHirachicalModelMatrix, newRadiant, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it only on y axis
			mn->inverseHirachicalModelMatrix = glm::translate(mn->inverseHirachicalModelMatrix, mn->position); // Translate rotated matrix bock to position
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