#include "BVG.hpp"
#include <iostream>

using namespace std;

btConvexHullShape* BVG::calculateVHACD(ModelNode* modelNode)
{
	vector<int>* indices = modelNode->getAllIndices(); // Array of vertex indexes (similar to an EBO)
	vector<float>* points = modelNode->getAllVertices();  // Array of coordinates, the vertices of the node or rather model

	IVHACD::Parameters    params; // V-HACD parameters: https://kmamou.blogspot.co.at/2014/12/v-hacd-20-parameters-description.html or in our own development source folder
	IVHACD* interfaceVHACD = CreateVHACD(); // create interface

	/*#if _DEBUG
		Callback callback;
		Logger   logger(LOG_FILE_PATH);
		params.m_logger = &logger;
		params.m_callback = &callback;
	#endif*/

	const unsigned int indicesStride = 3; // one index points to a vertex, 3 indices to a triangle
	const unsigned int pointStride = 3; // One vertex is ordered in this array as xyz, one trianle has 3 vertices which are equal to 9 entries in this array as xyzxyzxyz 
	const unsigned int indicesSize = indices->size() / indicesStride;
	const unsigned int pointSize = points->size() / pointStride;
	bool res = interfaceVHACD->Compute(points->data(), pointStride, pointSize, indices->data(), indicesStride, indicesSize, params); // compute approximate convex decomposition

	// read results
	unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls(); // Get the number of convex-hulls
	IVHACD::ConvexHull vhacdConvexHull;

	for (unsigned int p = 0; p < nConvexHulls; ++p)
		interfaceVHACD->GetConvexHull(p, vhacdConvexHull); // get the p-th convex-hull informatiion
														   
	btConvexHullShape* btShape = new btConvexHullShape((btScalar*)vhacdConvexHull.m_points, vhacdConvexHull.m_nPoints, sizeof(vhacdConvexHull.m_points[0]));// Constructor gives an LNK 2019 error do it with the hand. btConvexHullShape: the pointer to the first element of mesh vertices array, the total number of vertices and the stride between two vertices.

	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
	indices->clear();
	points->clear();
	delete indices;
	delete points;
	modelNode->indicesVerticesArray = false;

	return btShape;
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
void Logger::Log(const char* const msg)
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