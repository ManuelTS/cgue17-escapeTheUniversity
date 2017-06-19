#pragma once
#include "TransformationNode.hpp"

class CubeNode :
	public TransformationNode
{
public:
	CubeNode();
	~CubeNode();

	void draw() override;
};

