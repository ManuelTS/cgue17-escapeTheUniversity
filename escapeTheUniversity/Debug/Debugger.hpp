#pragma once

#include "../Shader.hpp"
#include <stdlib.h>
#include <sstream>
#include <vector>

class ModelLoader;
class LightNode;

class Debugger
{
private:
	unsigned int query;
	time_t cpuStart;

	std::string getAttachmentComponentType(int param);
	std::string getAttachmentObjectType(int param);
	std::string getAttachmentColorEncoding(int param);

	const std::string logFilePath = ".\\Setting\\";

	Shader* shadowDebug = nullptr; // pointer for the shadow debug renderer
	unsigned int quadVAO = 0; // shadow debug rendering quad on screen VAO
	unsigned int quadVBO = 0;
public:
	Debugger(void){}; // Private constructor to allow only one instance
	Debugger(Debugger const&); // Private constructor to prevent copies
	void operator=(Debugger const&); // Private constructor to prevent assignmentsDebugger();
	~Debugger();

	/*Returns the pointer to the unique instance of this class.*/
	static Debugger* Debugger::getInstance()
	{
		static Debugger instance;// lazy singleton, instantiated on first use
		return &instance;
	}

	void setDebugContext();
	void errorCallback(int error, const char* description);
	void pauseExit();
	void pauseExit(const char* msg);
	void pauseExit(std::string msg);
	void pause(const char* msg);
	void startGPUTimer();
	void stopGPUTimer();
	void startCPUTimer();
	void stopCPUTimer();
	void printFrameBufferAttachments();
	void checkWholeFramebufferCompleteness();
	void writeAllVertices(std::vector<float>* vertices, std::string fileNameWithoutEnding); // Writes all vectors to a log file
	void writeLogFile(std::string fileNameWithoutEnding, std::string text); // Writes a log file in the settings folder
	void renderShadowMap(float farPlane, unsigned int depthMapTextureHandle); // Renders a shadow map on screen
};

