#pragma once

#include "TransformationNode.hpp"

//AnimationNode name caused problems, this node does animation timings and updates and renders than the involved meshes
class AnimatNode : public TransformationNode
{
	private:
		double animationTime = 0.0; // The current animation time
		
	public:
		double timeAccumulator = 0.0; // Accumulates delta time until the next animation update is done

		AnimatNode() {}
		~AnimatNode() {}

		void draw() override; // Draws this node with animation by super class draw call
};

