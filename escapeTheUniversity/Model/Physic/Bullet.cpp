#include "Bullet.hpp"


Bullet::Bullet()
{
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new
		btDefaultCollisionConfiguration();
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();
	///the default constraint solver. For parallel processing you can use a different solver(see Extras / BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));


	// 
	// On position update of object do http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Detection&action=edit
	// Collision triggers: http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers&action=edit
	// Collision filtering, what object can collide with another one: http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Filtering
	// Broadphase: Use Sweep and prune due to fixed world size: http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Broadphase
	// Little about rigit bodies: http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Rigid_Bodies
}

/*
*/
btCollisionObject* Bullet::createBox(float mass)
{
	//btCollisionShape* colShape = new btBoxShape(btScalar(sx));
	//shapes.push_back(colShape);

	//btTransform startTransform;
	//startTransform.setIdentity();

	//btScalar    tMass(mass);

	////rigidbody is dynamic if and only if mass is non zero, otherwise static
	//bool isDynamic = (tMass != 0.f);

	//btVector3 localInertia(0, 0, 0);
	//if (isDynamic)
	//	colShape->calculateLocalInertia(tMass, localInertia);

	//startTransform.setOrigin(btVector3(px, py, pz));

	////using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	//btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	//btRigidBody::btRigidBodyConstructionInfo rbInfo(tMass, myMotionState, colShape, localInertia);
	//btRigidBody* body = new btRigidBody(rbInfo);
	//dynamicsWorld->addRigidBody(body);
	//return body;
	return nullptr;
}

void Bullet::step() {


	for (int i = 0; i<100; i++)
		{
		dynamicsWorld->stepSimulation(1.f / 60.f, 10);
		
		//print positions of all objects
		for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;

			if (body && body->getMotionState())
				body->getMotionState()->getWorldTransform(trans);
			else
				trans = obj->getWorldTransform();

		printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
			}
		}
}

Bullet::~Bullet()
{
	//remove the rigidbodies from the dynamics world and delete them
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j<shapes.size(); j++)
	{
		btCollisionShape* shape = shapes[j];
		shapes[j] = 0;
		delete shape;
	}
	//delete dynamics world
	delete dynamicsWorld;
	//delete solver
	delete solver;
	//delete broadphase
	delete overlappingPairCache;
	//delete dispatcher
	delete dispatcher;
	delete collisionConfiguration;
	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	shapes.clear();
}