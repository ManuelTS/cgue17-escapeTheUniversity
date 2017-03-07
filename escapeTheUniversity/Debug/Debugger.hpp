#pragma once

#include <stdlib.h>
#include <sstream>

class Debugger
{
private:
	unsigned int query;
	time_t cpuStart;

	std::string getAttachmentComponentType(int param);
	std::string getAttachmentObjectType(int param);
	std::string getAttachmentColorEncoding(int param);
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
};

