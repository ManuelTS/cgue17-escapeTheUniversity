#pragma once
#define degreesToRadians(x) x*(3.141592f/180.0f)
#define GLM_FORCE_RADIANS
#include "Node.hpp"

class TransformationNode : public Node
{
	private:
		const float RADIANT = degreesToRadians(90.0f); // Value is in angle, result in radiants
		const float MAX_ROTATION_RADIANT = degreesToRadians(140.0f); // Value is in angle, result in radiants
		float currentRotationRadiant = 0.0f;
		bool transform = false;
		float change(bool plus); // Transforms all children, plus = true if a positive value, false if a negative value is used
	public:
		TransformationNode();
		~TransformationNode();

		void switchState(); // Switches the state of this transformation node
		void draw() override; // Draws this nodes
};

