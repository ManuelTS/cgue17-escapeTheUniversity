#pragma once
#include "v-hacd\public\VHACD.h"
#include "..\Node\Node.hpp"
#include "..\Node\ModelNode.hpp"
#include <string>
#include <fstream>
#include <future>

using namespace VHACD;

class Node;

// Bounding Volume Generation with V-Hierarchical Approximate Convex Decomposition (V-HACD)
class BVG {
private:
	const std::string LOG_FILE_PATH = "Setting\vhacd.log";
	const unsigned int concurentThreadsSupported = std::thread::hardware_concurrency(); // Get number of hardware thread contexts (on most systems its CPU count which is != to possible thread count) but who cares? we just want a simple speedup

	bool calculateVHACD(ModelNode* modelNode); // Calculates an V-Hierarchical Approximate Convex Decomposition (V-HACD) bounding volume, true is only retured because threads need a state to check or wait for
	void removeFinished(vector<future<bool>>* threads); // Removes all finished threads from the argument vector
public:
	void calculateBoundingShapes(Node* current);
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