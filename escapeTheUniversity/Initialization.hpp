#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

using namespace std;
/*Read ini file and loads the shaders. Open Window, Initialize OpenGL, Load Extensions, 
Load Content (Models, Textures, Shader).*/
class Initialization{
public:
	int width = 800; // Standard values, will be overriden by ini file
	int height = 600;
	int maxWidth = 1920;
	int maxHeight = 1080;
	int fps = 60;
	bool fullscreen = false;
	char* windowTitle = "Escape the University";
	double mouseSensitivity = 0.0;
	double movingSpeed = 0.0;
	double scrollSpeed = 0.0;
	double zoom = 45.0;

	Initialization();
	~Initialization();

private:
	const string inifile = ".\\Setting\\StartOptions.ini"; // Ini file location and name

	void readIni();
};