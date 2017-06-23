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
	const float DEFAULT_COLLISION_MARGIN = 0.05;
	const unsigned int CONCURENT_THREADS_SUPPORTED = std::thread::hardware_concurrency(); // Get number of hardware thread contexts (on most systems its CPU count which is != to possible thread count) but who cares? we just want a simple speedup
	vector<future<bool>>* threads = new vector<future<bool>>(); // Contains all futures for collision object generation

	btDefaultCollisionConfiguration* collisionConfiguration;//collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btCollisionDispatcher* dispatcher;//use the default collision dispatcher. For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	btBroadphaseInterface* overlappingPairCache;//btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btSequentialImpulseConstraintSolver* solver;//the default constraint solver. For parallel processing you can use a different solver(see Extras / BulletMultiThreaded)
	btDiscreteDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> shapes; // All generated shapes
	BulletDebugDrawer* debugDrawer; // Own implemented debug drawer

	Bullet(void) {}; // Private constructor to allow only one instance
	Bullet(Bullet const&); // Private constructor to prevent copies
	void operator=(Bullet const&); // Private constructor to prevent assignments

	void removeFinished(vector<future<bool>>* threads); // Removes all finished threads from the argument vector
	bool distributeBoundingGeneration(ModelNode* mn); // Chooses the correct method to calculate the bounding volume for this model node
	void createBuilding(ModelNode* mn); // Creates a big static btBvhTriangleMeshShape 
	btConvexHullShape* pureBulletConvexHullGeneration(ModelNode* mn); // Puts all points into bullet and generates an optimized convex hull from it
	void removeScaleMatrix(glm::mat4 matrix, btCollisionShape* shape, btTransform* trans); // Removes the scaling from the matrix and applies it afterwards with the setLocalScaling method and sets the matrix without Scaling as openGLMatrix in trans
public:
	const unsigned int FLAG_LOCATION = 256; // Flag to use texture or material color
	/*Returns the pointer to the unique instance of this class.*/
	static Bullet* Bullet::getInstance()
	{
		static Bullet instance;// lazy singleton, instantiated on first use
		return &instance;
	}
	~Bullet();

	void init(); // Initalizes the bullet world, called only once
	void createAndAddBoundingObjects(Node* current); // Creates and adds the bounding volumes into the bullet world and to the nodes
	void createDoorHinge(ModelNode* mn); //creates a hinge between door + angle, whereas angle is immovable standallone
	void join();// Waits and removes all threads that where created for collision object creation
	void createCamera(Camera* camera); // Creates the bounding object for the camera
	btDiscreteDynamicsWorld* getDynamicsWorld();
	void Bullet::debugDraw(); // Draws the whole debug world of bullet, see constructor and BulletDebugDraw.cpp and .hpp
	btCollisionObject* createBox(float mass); // Creates a bounding box
};

