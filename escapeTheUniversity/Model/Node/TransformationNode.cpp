#pragma once

#include "../../SoundManager.hpp"
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
	{
		//SoundManager::getInstance()->playSound("close_interior_wood_door.mp3"); // only display sound if only one door is open or closed!
		newRadiant = -newRadiant;
	}
	else
		//SoundManager::getInstance()->playSound("open_interior_wood_door_with_squeak.mp3");

	for (Node* child : children)
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(child);
		ModelNode* mnParent = dynamic_cast<ModelNode*>(mn->parent);

		if (mn)
		{
			glm::vec3 pos = mn->position;
			mn->hirachicalModelMatrix = glm::translate(mn->hirachicalModelMatrix, -pos); // Translate to origin to rotate there
			mn->hirachicalModelMatrix = glm::rotate(mn->hirachicalModelMatrix, newRadiant, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it only on y axis
			mn->hirachicalModelMatrix = glm::translate(mn->hirachicalModelMatrix, pos); // Translate rotated matrix bock to position
			// - rotate is faster than inverseTranspose
			//pos = glm::vec3(hirachicalModelMatrix[3]);
			mn->inverseHirachicalModelMatrix = glm::translate(mn->inverseHirachicalModelMatrix, -pos); // Translate to origin to rotate there
			mn->inverseHirachicalModelMatrix = glm::rotate(mn->inverseHirachicalModelMatrix, newRadiant, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate it only on y axis
			mn->inverseHirachicalModelMatrix = glm::translate(mn->inverseHirachicalModelMatrix, pos); // Translate rotated matrix bock to position
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
