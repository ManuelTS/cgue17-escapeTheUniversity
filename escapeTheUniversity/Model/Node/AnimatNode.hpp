#pragma once

#include "TransformationNode.hpp"

//AnimationNode name causec problems
class AnimatNode : public TransformationNode
{
	private:
		double animationTime = 0.0; // Has the current animation time in it
		
	public:
		double timeAccumulator = 0.0; // Accumulates delta time until the next animation update is done

		AnimatNode() {}
		~AnimatNode() {}

		void draw() override; // Draws this node with animation
};

