#pragma once
#include "Node.hpp"

class TransformationNode : public Node
{
private:
	const float ANGLE = glm::radians(0.01f);
	const float MAX_ROTATION = glm::radians(140.0f);
	float currentRotation = 0.0f;
	bool transform = false;
public:
	TransformationNode();
	~TransformationNode();

	void switchState();
	void draw() override;
};

