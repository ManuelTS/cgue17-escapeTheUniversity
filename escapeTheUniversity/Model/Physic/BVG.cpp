#include "BVG.hpp"
#include <iostream>
#include <chrono>
#include "../../Debug/MemoryLeakTracker.h"

using namespace std;

void BVG::calculateBoundingShapes(Node* current)
{
	if (true &&concurentThreadsSupported == 0) // Hyperthreading unsupported, calculate normally
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);

		if (mn && mn->meshes.size() > 0)
			calculateVHACD(mn);

		for (Node* child : current->children)
			calculateBoundingShapes(child);
	}
	else  // Hyperthreaded bounding volume generation
	{
		vector<Node*> nodes = current->getAllNodesDepthFirst(current);
		vector<future<bool>>* threads = new vector<future<bool>>();

		for (Node* child : nodes)
		{
			ModelNode* mn = dynamic_cast<ModelNode*>(child);

			if (mn && mn->meshes.size() > 0)
			{ // Leave if structure like this
				if (threads->size() < concurentThreadsSupported) // If the system thread maximum is not reached, create a thread and calc bounding volume
					threads->push_back(async(launch::async, &BVG::calculateVHACD, this, mn));
				else // All threads busy, wait for one to finish and if so remove him
					while (threads->size() >= concurentThreadsSupported)
						removeFinished(threads);
			}
		}

		while (threads->size() > 0) // Wait until all remaining threads are finished and remove them
			removeFinished(threads);

		nodes.clear();
		delete threads;
	}
}

void BVG::removeFinished(vector<future<bool>>* threads)
{
	for(std::vector<future<bool>>::iterator i = threads->begin(); i != threads->end(); i++) // algorithm::remove_if caused problems
		if (i->valid() && i->get()) // Valid checks if the future has a shared state , get gets true ergo thread finished, delete it 
		{
			threads->erase(i);
			break;
		}
}

bool BVG::calculateVHACD(ModelNode* modelNode)
{
	vector<int>* triangles = modelNode->getAllIndices(); // Array of vertex indexes (similar to an EBO)
	vector<float>* points = modelNode->getAllVertices();  // Array of coordinates, the vertices of the node or rather model

	IVHACD::Parameters    params; // V-HACD parameters
	IVHACD* interfaceVHACD = CreateVHACD(); // create interface

	#if _DEBUG
		Callback callback;
		Logger   logger(LOG_FILE_PATH);
		params.m_logger = &logger;
		params.m_callback = &callback;
	#endif

	const unsigned int triangleStride = 3; // one index points to a vertex, 3 indices to a triangle
	const unsigned int pointStride = 3; // One vertex is ordered in this array as xyz, one trianle has 3 vertices which are equal to 9 entries in this array as xyzxyzxyz 
	bool res = interfaceVHACD->Compute(points->data(), pointStride, points->size() / pointStride, triangles->data(), triangleStride, triangles->size() / triangleStride, params); // compute approximate convex decomposition

	// read results
	unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls(); // Get the number of convex-hulls
	IVHACD::ConvexHull ch;

	for (unsigned int p = 0; p < nConvexHulls; ++p)
	{
		interfaceVHACD->GetConvexHull(p, ch); // get the p-th convex-hull information
		
		/*#if _DEBUG
			for (unsigned int v = 0, idx = 0; v < ch.m_nPoints; ++v, idx += 3)
				printf("x=%f, y=%f, z=%f", ch.m_points[idx], ch.m_points[idx + 1], ch.m_points[idx + 2]);
			for (unsigned int t = 0, idx = 0; t < ch.m_nTriangles; ++t, idx += 3)
				printf("i=%f, j=%f, k=%f", ch.m_triangles[idx], ch.m_triangles[idx + 1], ch.m_triangles[idx + 2]);
		#endif*/
	}

	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
	triangles->clear();
	points->clear();
	delete triangles;
	delete points;

	// TODO Put the ConvexHull in bullet::btCompoundShape or btConvexHullShape
	// test hulls with http://www.bulletphysics.org/mediawiki-1.5.8/index.php/BtShapeHull_vertex_reduction_utility
	// TODO link hull pointer with node
	return true;
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