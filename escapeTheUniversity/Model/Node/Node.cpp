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
