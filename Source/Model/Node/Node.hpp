#pragma once

#include <vector>

class Node
{
public:
	Node* parent; //Root node has pointer as parent it iself
	std::vector<Node*>children; // Children of this node
	std::string name;

	Node();
	~Node();

	virtual void draw();
	virtual bool isLeaf();
	std::vector<Node*> getAllNodesDepthFirst(Node* current); //Depth first traverses the tree and returns a pre ordered node vector
};

