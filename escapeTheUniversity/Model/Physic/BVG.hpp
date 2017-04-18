#pragma once
#include "v-hacd\public\VHACD.h"
#include <string>
#include <fstream>

using namespace VHACD;

// Bounding Volume Generation with V-Hierarchical Approximate Convex Decomposition (V-HACD)
class BVG {
private:
	const std::string LOG_FILE_PATH = "Setting\vhacd.log";

public:
	void calculateVHACD();
};

class Callback : public IVHACD::IUserCallback
{
public:
	Callback(void);
	~Callback();
	void Update(const double overallProgress, const double stageProgress, const double operationProgress, const char* const stage, const char* const operation);
};

class Logger : public IVHACD::IUserLogger
{
public:
	Logger(void);
	Logger(const std::string & fileName);
	~Logger();
	void Log(const char * const msg);
	void OpenFile(const std::string & fileName);
private:
	std::ofstream m_file;
};