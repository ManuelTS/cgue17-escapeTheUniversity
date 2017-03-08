#include "Shader.hpp"
#include <iostream>
#include <fstream>
#include "Debug\Debugger.hpp"
#include "Debug\MemoryLeakTracker.h"

using namespace std;

/*Creates a program of the vertex and fragement shader.*/
Shader::Shader(const string& vertexShader, const string& fragmentShader)
{
	programHandle = glCreateProgram();

	check(programHandle, "program");

	loadShader(SHADER_DIR + vertexShader, GL_VERTEX_SHADER, vertexHandle);
	loadShader(SHADER_DIR + fragmentShader, GL_FRAGMENT_SHADER, fragmentHandle);
	link();
}

/*Deletes all used shaders.*/
Shader::~Shader()
{
	glDeleteProgram(programHandle);
}

/*Checks if the handle has another value than zero, if not displays an error message.*/
void Shader::check(GLuint handle, const string& name){
	if (handle == 0)
		Debugger::getInstance()->pauseExit("Invalid " + name + " handle value");
}

/*Uses the current program and causes a state change.*/
void Shader::useProgram() const
{
	glUseProgram(programHandle);
}

/*Loads the shader, sets its source, compiles it and logs errors. Handle receives the internal openGL number.*/
void Shader::loadShader(const string& shader, GLenum shaderType, GLuint& handle)
{
	ifstream shaderFile(shader);

	if (shaderFile.good())
	{
		string code = string(istreambuf_iterator<char>(shaderFile), istreambuf_iterator<char>());

		shaderFile.close();
		handle = glCreateShader(shaderType);

		check(handle, "shader");

		const char* codePtr = code.c_str();
		glShaderSource(handle, 1, &codePtr, nullptr);
		glCompileShader(handle);

		//Test if compilation was successfull
		GLint succeded;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &succeded);

		if (!succeded)// Unsuccessful, read log
		{
			GLint logSize;
			glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);
			GLchar* message = new GLchar[logSize];
			glGetShaderInfoLog(handle, logSize, nullptr, message);
			Debugger::getInstance()->pauseExit(message);
			delete[] message;
		}
	}
	else
		Debugger::getInstance()->pauseExit("Failed to open file" + shader); //Fehlermeldung wenn shaderFile nicht geöffnet werden konnte
}

/*Links the fragement and vertex shader with the program and logs errors.*/
void Shader::link()
{
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);
	glLinkProgram(programHandle);

	GLint succeded;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &succeded);

	if (!succeded)
	{
		GLint logSize;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logSize);

		GLchar* message = new GLchar[logSize];
		glGetProgramInfoLog(programHandle, logSize, nullptr, message);
		Debugger::getInstance()->pauseExit(message);
		delete[] message;
	}

	glDeleteShader(vertexHandle); // Delete shaders, program has 'em now
	glDeleteShader(fragmentHandle);
}