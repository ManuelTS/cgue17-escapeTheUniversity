#include "CubeNode.hpp"

CubeNode::CubeNode(){}


CubeNode::~CubeNode(){}

void CubeNode::draw()
{
	const float newRadiant = Frustum::getInstance()->degreesToRadians(45.0f) * RenderLoop::getInstance()->getTimeDelta();
			
	hirachicalModelMatrix = glm::translate(hirachicalModelMatrix, -position); // Translate to origin to rotate there
	hirachicalModelMatrix = glm::rotate(hirachicalModelMatrix, newRadiant, glm::vec3(0.5f, 1.0f, 0.5f));// Rotate it only on y axis
	hirachicalModelMatrix = glm::translate(hirachicalModelMatrix, position); // Translate rotated matrix bock to glm::vec4(position, 1.0f)ition
																				// - rotate is faster than inverseTransglm::vec4(position, 1.0f)e
	inverseHirachicalModelMatrix = glm::translate(inverseHirachicalModelMatrix, -position); // Translate to origin to rotate there
	inverseHirachicalModelMatrix = glm::rotate(inverseHirachicalModelMatrix, newRadiant, glm::vec3(0.5f, 1.0f, 0.5f));// Rotate it only on y axis
	inverseHirachicalModelMatrix = glm::translate(inverseHirachicalModelMatrix, position); // Translate rotated matrix bock to glm::vec4(position, 1.0f)ition

	ModelNode::draw();
}
