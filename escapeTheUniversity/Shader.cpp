#include "Shader.hpp"
#include "Debug\Debugger.hpp"
#include "Debug\MemoryLeakTracker.h"

using namespace std;

/*Creates a program of the vertex and fragement shader.*/
Shader::Shader(const char* shader)
{
	programHandle = glCreateProgram();
	check(programHandle, "program");

	if (strcmp(shader, "text") == 0)
	{
		loadShader(TEXT_VERT, GL_VERTEX_SHADER, vertexHandle);
		loadShader(TEXT_FRAG, GL_FRAGMENT_SHADER, fragmentHandle);
	}
	else if (strcmp(shader, "gBuffer") == 0)
	{
		loadShader(GBUFFER_VERT, GL_VERTEX_SHADER, vertexHandle);
		loadShader(GBUFFER_FRAG, GL_FRAGMENT_SHADER, fragmentHandle);
	}
	else if (strcmp(shader, "deferredShading") == 0)
	{
		loadShader(DEFERRED_SHADING_VERT, GL_VERTEX_SHADER, vertexHandle);
		loadShader(DEFERRED_SHADING_FRAG, GL_FRAGMENT_SHADER, fragmentHandle);
	}
	else if (strcmp(shader, "image") == 0)
	{
		loadShader(IMAGE_VERT, GL_VERTEX_SHADER, vertexHandle);
		loadShader(IMAGE_FRAG, GL_FRAGMENT_SHADER, fragmentHandle);
	}
	else
		Debugger::getInstance()->pauseExit("Unknown shader type");
	
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
void Shader::loadShader(const char* shader, GLenum shaderType, GLuint& handle)
{
		handle = glCreateShader(shaderType);
		check(handle, "shader");

		glShaderSource(handle, 1, &shader, nullptr);
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