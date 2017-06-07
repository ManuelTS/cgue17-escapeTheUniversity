#pragma once
#include "../Node/ModelNode.hpp"
#include "Bullet/btBulletDynamicsCommon.h"
#include "../../Debug/BulletDebugDrawer.hpp"
#include <future>

using namespace std;

class Camera;

/*This class is the link to the bullet framework.*/
class Bullet
{
private:
	bool initialized = false; // True if bullet wase initialized once, flase if not. Default: false
	const unsigned int concurentThreadsSupported = std::thread::hardware_concurrency(); // Get number of hardware thread contexts (on most systems its CPU count which is != to possible thread count) but who cares? we just want a simple speedup

	//collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration;
	//use the default collision dispatcher. For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;
	//btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;
	//the default constraint solver. For parallel processing you can use a different solver(see Extras / BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> shapes;
	BulletDebugDrawer* debugDrawer; // Own implemented debug drawer

	Bullet(void) {}; // Private constructor to allow only one instance
	Bullet(Bullet const&); // Private constructor to prevent copies
	void operator=(Bullet const&); // Private constructor to prevent assignments

	void removeFinished(vector<future<bool>>* threads); // Removes all finished threads from the argument vector
	bool distributeBoundingGeneration(ModelNode* mn); // Chooses the correct method to calculate the bounding volume for this model node
	void Bullet::createBuilding(ModelNode* mn); // Creates a big static btBvhTriangleMeshShape 
public:
	/*Returns the pointer to the unique instance of this class.*/
	static Bullet* Bullet::getInstance()
	{
		static Bullet instance;// lazy singleton, instantiated on first use
		return &instance;
	}
	~Bullet();

	void init(); // Initalizes the bullet world, called only once
	void createAndAddBoundingObjects(Node* current); // Creates and adds the bounding volumes into the bullet world and to the nodes
	void createCamera(Camera* camera); // Creates the bounding object for the camera
	void step(const double deltaTime); // Sets the timing syncronization of bullet physics
	void Bullet::debugDraw(); // Draws the whole debug world of bullet, see constructor and BulletDebugDraw.cpp and .hpp
	btCollisionObject* createBox(float mass); // Creates a bounding box
};

