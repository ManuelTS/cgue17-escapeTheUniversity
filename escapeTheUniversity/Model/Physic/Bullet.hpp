#pragma once
#include "../Node/ModelNode.hpp"
#include "Bullet/btBulletDynamicsCommon.h"
#include <future>

using namespace std;

/*This class is the link to the bullet framework.*/
class Bullet
{
private:
	bool initialized = false; // True if bullet wase initialized once, flase if not. Default: false
	const unsigned int concurentThreadsSupported = std::thread::hardware_concurrency(); // Get number of hardware thread contexts (on most systems its CPU count which is != to possible thread count) but who cares? we just want a simple speedup

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration;
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;
	///the default constraint solver. For parallel processing you can use a different solver(see Extras / BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> shapes;

	Bullet(void) {}; // Private constructor to allow only one instance
	Bullet(Bullet const&); // Private constructor to prevent copies
	void operator=(Bullet const&); // Private constructor to prevent assignments

	void removeFinished(vector<future<bool>>* threads); // Removes all finished threads from the argument vector
	bool distributeBoundingGeneration(ModelNode* mn); // Chooses the correct method to calculate the bounding volume for this model node
	void calculatePlane(ModelNode* mn); // Plane calculation and adding to node and bullet world
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
	void step();
	btCollisionObject* createBox(float mass); // Creates a bounding box
};

