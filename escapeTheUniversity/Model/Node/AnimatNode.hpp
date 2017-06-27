#pragma once

#include "TransformationNode.hpp"

//AnimationNode name caused problems, this node does animation timings and updates and renders than the involved meshes
class AnimatNode : public TransformationNode
{
	private:
		double animationTime = 0.0; // The current animation time
		glm::vec3 walkPoint1 = glm::vec3(-5.7f, -0.3275f, 12.0f);
		glm::vec3 walkPoint2 = glm::vec3(20.5f, -0.3275f, 12.0f);
		glm::vec3 walkPoint3 = glm::vec3(20.5f, -0.3275f, -13.75f);
		glm::vec3 walkPoint4 = glm::vec3(-5.7f, -0.3275f, -13.75f);

		/*Defines the movestep of the Character*/
		const float MOVESTEP = 0.06f;

		void moveEnemy(); //moves the enemy along the walkingpoints


	public:
		double timeAccumulator = 0.0; // Accumulates delta time until the next animation update is done

		AnimatNode() {}
		~AnimatNode() {}


		bool endOf1Reached = false; //inidicates that we have to turn and go over to the next point (2)
		bool endOf2Reached = false; //inidicates that we have to turn and go over to the next point (3)
		bool endOf3Reached = false; //inidicates that we have to turn and go over to the next point (4)
		bool endOf4Reached = false; //inidicates that we have to turn and go over to the next point (1) 

		void draw() override; // Draws this node with animation by super class draw call
};

