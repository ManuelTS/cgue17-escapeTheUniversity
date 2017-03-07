#include "RenderLoop.hpp"

int main(int argc, char** argv)
{
	RenderLoop::getInstance()->start();

	exit(EXIT_SUCCESS);
}