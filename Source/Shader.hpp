#pragma once

#include <string>
#include <GL\glew.h>

class Shader
{
public:
	// gBufferShader location constants
	const int viewLocation = 8; // gBuffer.vert
	const int projectionLocation = 12; // gBuffer.vert
	// deferredShader location constants
	const int viewPositionLocation = 0; // defferredShader.frag

	unsigned int programHandle; // Shader handle

	Shader(const std::string& vertexShader, const std::string& fragementShader);
	~Shader();

	void useProgram() const;
private:
	unsigned int vertexHandle;
	unsigned int fragmentHandle;

	void loadShader(const std::string& shader, GLenum shaderType, unsigned int& handle);
	void link();
	void check(unsigned int handle, const std::string& name);
};
