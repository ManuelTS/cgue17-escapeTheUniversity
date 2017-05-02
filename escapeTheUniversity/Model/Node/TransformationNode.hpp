#pragma once
#include "Node.hpp"

class TransformationNode : public Node
{
private:
	const float ANGLE = glm::radians(0.01f);
	const float MAX_ROTATION = glm::radians(140.0f);
	float currentRotation = 0.0f;
	bool transform = false;
	float change(bool plus); // Transforms all children, plus = true if a positive value, false if a negative value is used
public:
	TransformationNode();
	~TransformationNode();

	void switchState(); // Switches the state of this transformation node
	void draw() override; // Draws this nodes
};

