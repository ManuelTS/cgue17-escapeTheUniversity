#include "BVG.hpp"
#include <iostream>

using namespace std;

void BVG::calculateVHACD()
{
	int * triangles; // array of indexes
	float * points;  // array of coordinates 
			
	// load the mesh 
	
	IVHACD::Parameters    params; // V-HACD parameters
	IVHACD * interfaceVHACD = CreateVHACD(); // create interface

	#if _DEBUG
		Callback callback;
		Logger   logger(LOG_FILE_PATH);
		params.m_logger = &logger;
		params.m_callback = &callback;
	#endif

	bool res = interfaceVHACD->Compute(points, 3, nPoints, triangles, 3, nTriangles, params); // compute approximate convex decomposition

	// read results
	unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls(); // Get the number of convex-hulls
	IVHACD::ConvexHull ch;

	for (unsigned int p = 0; p < nConvexHulls; ++p)
	{
		interfaceVHACD->GetConvexHull(p, ch); // get the p-th convex-hull information
		for (unsigned int v = 0, idx = 0; v < ch.m_nPoints; ++v, idx += 3)
			printf("x=%f, y=%f, z=%f", ch.m_points[idx], ch.m_points[idx + 1], ch.m_points[idx + 2]);
		for (unsigned int t = 0, idx = 0; t < ch.m_nTriangles; ++t, idx += 3)
			printf("i=%f, j=%f, k=%f", ch.m_triangles[idx], ch.m_triangles[idx + 1], ch.m_triangles[idx + 2]);
	}

	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
	// Put the ConvexHull in bullet::btCompoundShape or btConvexHullShape
	// test hulls with http://www.bulletphysics.org/mediawiki-1.5.8/index.php/BtShapeHull_vertex_reduction_utility
}

Callback::Callback(void) {}
Callback::~Callback() {}

void Callback::Update(const double overallProgress,const double stageProgress,const double operationProgress, const char* const stage, const char* const operation)
{
		cout << (int)(overallProgress + 0.5) << "% "
			<< "[ " << stage << " " << (int)(stageProgress + 0.5) << "% ] "
			<< operation << " " << (int)(operationProgress + 0.5) << "%" << endl;
};

Logger::Logger(void) {}
Logger::Logger(const std::string & fileName) { 
	OpenFile(fileName);
}
Logger::~Logger() {}
void Logger::Log(const char * const msg)
{
	if (m_file.is_open())
	{
		m_file << msg;
		m_file.flush();
	}
};
void Logger::OpenFile(const string & fileName) { 
	m_file.open(fileName.c_str());
}