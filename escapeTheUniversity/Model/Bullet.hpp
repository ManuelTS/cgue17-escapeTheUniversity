#pragma once
#include "Bullet/btBulletDynamicsCommon.h"

/*This class is the link to the bullet framework.*/
class Bullet
{
private:
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
public:
	Bullet();
	~Bullet();

	void step();
	btCollisionObject* createBox(); // Creates a bounding box
};

