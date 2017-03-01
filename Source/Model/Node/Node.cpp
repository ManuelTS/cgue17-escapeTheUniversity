#include "Node.hpp"

using namespace std;

Node::Node()
{
}


Node::~Node()
{
}

void Node::draw(){}

bool Node::isLeaf()
{
	return children.size() < 1;
}

/*Depth first traverses the tree and returns a pre ordered node vector.*/
vector<Node*> Node::getAllNodesDepthFirst(Node* current)
{
	vector<Node*> children;

	for (unsigned int i = 0; current != nullptr && i < current->children.size(); i++)
	{
		Node* child = current->children[i];
		children.push_back(child);
		vector<Node*> ancestors = getAllNodesDepthFirst(child);

		if (ancestors.size() > 0)
			children.insert(children.end(), ancestors.begin(), ancestors.end());
	}

	return children;
}