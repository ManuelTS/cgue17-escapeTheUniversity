#include "AnimatNode.hpp"
#include "../ModelLoader.hpp"

void AnimatNode::draw()
{
	if (timeAccumulator > 0.03)
	{
		// Argument is here the time inside the animation, not time delta!
		Animator* animator = ModelLoader::getInstance()->animator;
		animator->UpdateAnimation(animationTime += 0.001, animator->ANIMATION_TICKS_PER_SECOND);
		timeAccumulator = 0.0;
	}
	
	ModelNode::draw();
}