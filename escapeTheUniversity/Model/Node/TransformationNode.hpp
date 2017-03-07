#pragma once
#include "Node.hpp"

class TransformationNode : public Node
{
private:
	const float ANGLE = 0.0015;
	const float MAX_ROTATION = 0.010;
	float currentRotation = 0.0;
	bool transform = false;
public:
	TransformationNode();
	~TransformationNode();

	void switchState();
	void draw() override;
};

