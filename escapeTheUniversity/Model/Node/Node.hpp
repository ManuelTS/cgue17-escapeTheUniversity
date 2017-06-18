#pragma once

#include <vector>

class Node
{
public:
	Node* parent = nullptr; //Root node has pointer as parent it iself
	std::vector<Node*>children; // Children of this node
	std::string name; // of the node

	Node();
	~Node();

	virtual void draw();
	virtual bool isLeaf();
};

