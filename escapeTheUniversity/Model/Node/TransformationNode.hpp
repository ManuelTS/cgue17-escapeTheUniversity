#pragma once
#include "Node.hpp"

class TransformationNode : public Node
{
private:
	const float ANGLE = 0.010f;
	const float MAX_ROTATION = 0.0070f;
	float currentRotation = 0.0f;
	bool transform = false;
public:
	TransformationNode();
	~TransformationNode();

	void switchState();
	void draw() override;
};

