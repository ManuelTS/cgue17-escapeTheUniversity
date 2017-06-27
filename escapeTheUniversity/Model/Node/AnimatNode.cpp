#include "AnimatNode.hpp"
#include "../ModelLoader.hpp"

void AnimatNode::draw()
{
	if (timeAccumulator > 0.03)
	{
		Animator* animator = ModelLoader::getInstance()->animator;
		animator->UpdateAnimation(animationTime += 0.001, animator->ANIMATION_TICKS_PER_SECOND);// Argument is here the time inside the animation, not time delta!
		timeAccumulator = 0.0;


		
		glm::vec3 pos = ModelNode::position;
		pos.z = pos.z + 0.05;
		this->rigidBody->translate(btVector3(pos.x, pos.y, pos.z)); //put her into the floor
		//ModelNode::hirachicalModelMatrix = glm::translate(ModelNode::hirachicalModelMatrix, pos); // Translate to origin to rotate there							
	    //ModelNode::inverseHirachicalModelMatrix = glm::translate(ModelNode::inverseHirachicalModelMatrix, pos); // Translate to origin to rotate there

	}
	
	ModelNode::draw();
}