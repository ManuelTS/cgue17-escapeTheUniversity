#include "Bullet.hpp"
#include "BVG.hpp"
#include "../ModelLoader.hpp"
#include "../../Camera/Camera.hpp"
//#include "../../Debug/MemoryLeakTracker.h" // Not possible in here because of "solver = new btSequentialImpulseConstraintSolver;"

void Bullet::init()
{
	if (!initialized) // Flag for singelton, init bullet world only once
	{
		//collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
		collisionConfiguration = new btDefaultCollisionConfiguration();
		//use the default collision dispatcher. For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		//btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
		overlappingPairCache = new btDbvtBroadphase();
		//the default constraint solver. For parallel processing you can use a different solver(see Extras / BulletMultiThreaded)
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -10, 0));
		#if _DEBUG
			debugDrawer = new BulletDebugDrawer();
			debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
			dynamicsWorld->setDebugDrawer(debugDrawer);
		#endif

		initialized = !initialized;
	}
}

void Bullet::createAndAddBoundingObjects(Node* current)
{
	if (true)  //concurentThreadsSupported == 0) // Hyperthreading unsupported, calculate normally
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);

		if (mn && mn->meshes.size() > 0)
			distributeBoundingGeneration(mn);

		for (Node* child : current->children)
			createAndAddBoundingObjects(child);
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
					threads->push_back(async(launch::async, &Bullet::distributeBoundingGeneration, this, mn));
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

void Bullet::removeFinished(vector<future<bool>>* threads)
{
	for (std::vector<future<bool>>::iterator i = threads->begin(); i != threads->end(); i++) // algorithm::remove_if caused problems
		if (i->valid() && i->get()) // Valid checks if the future has a shared state , get gets true ergo thread finished, delete it 
		{
			threads->erase(i);
			break;
		}
}

bool Bullet::distributeBoundingGeneration(ModelNode* mn)
{
	if (mn->bounding)
	{
		BVG* bvg = new BVG();
		btConvexHullShape* shape = bvg->calculateVHACD(mn); 
		delete bvg;
		// Max 100 vertices
		//shape->optimizeConvexHull();
		#if _DEBUG
			//shape->initializePolyhedralFeatures(); // Changing the collision shape now bad idea,  That will make the debug rendering more pretty, but doesn't change anything related to collision detection etc. http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=11385&p=38354&hilit=initializePolyhedralFeatures#p38354
		#endif
		// test hulls with http://www.bulletphysics.org/mediawiki-1.5.8/index.php/BtShapeHull_vertex_reduction_utility
		const float mass = 5.0;
		btVector3 localInertia = btVector3(0, 0, 0);
		shape->calculateLocalInertia(mass, localInertia);
		shape->setMargin(0.05f);
		glm::vec3 pos = mn->getWorldPosition();
		btDefaultMotionState* myMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z)));
		btRigidBody::btRigidBodyConstructionInfo ci(mass, myMotionState, shape, localInertia);
		btRigidBody* rB = new btRigidBody(ci);
		mn->collisionObject = rB;
		dynamicsWorld->addRigidBody(rB);
	}
	else if (mn->name.find(ModelLoader::getInstance()->LEFT_WING) != string::npos || mn->name.find(ModelLoader::getInstance()->RIGHT_WING) != string::npos)
		createBuilding(mn);

	return true;
}

void Bullet::createBuilding(ModelNode* mn) {
	vector<int>* indices = mn->getAllIndices(); // Array of vertex indexes (similar to an EBO)
	vector<float>* points = mn->getAllVertices();  // Array of coordinates, the vertices of the node or rather model

	btTriangleMesh *mesh= new btTriangleMesh();

	// copy vertices into mesh
	btVector3 v1; // Vertices of one triangle
	btVector3 v2;
	btVector3 v3;
	for (int i = 0; i < indices->size(); i++) // Loop through all indices, fil v1-3 from them to create one triangle
	{
		const unsigned int vertexIndex = indices->at(i);
		mesh->addIndex(vertexIndex);

		switch (i % 3)
		{
			case 0:
				v1.setX(points->at(vertexIndex));
				v1.setY(points->at(vertexIndex + 1));
				v1.setZ(points->at(vertexIndex + 2));
				break;
			case 1:
				v2.setX(points->at(vertexIndex));
				v2.setY(points->at(vertexIndex + 1));
				v2.setZ(points->at(vertexIndex + 2));
				break;
			case 2: 
				v3.setX(points->at(vertexIndex));
				v3.setY(points->at(vertexIndex + 1));
				v3.setZ(points->at(vertexIndex + 2));
				mesh->addTriangle(v1, v2, v3); // All three triangle vertices are complete copy them into the mesh
				break;
		}
	}

	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(mesh, true, true);
	shape->setMargin(0.05f);
	glm::vec3 pos = mn->getWorldPosition();
	mn->collisionObject = new btCollisionObject(); // Use btCollisionObject since a btRigitBody is just a subclass with mass and inertia which is not needed here
	mn->collisionObject->setCollisionShape(shape);
	
	btTransform trans;
	trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
	mn->collisionObject->setWorldTransform(trans);
	dynamicsWorld->addCollisionObject(mn->collisionObject);
}

void Bullet::createCamera(Camera* c) 
{
	btCollisionShape* shape= new btCylinderShape(btVector3(1, 2, 1)); 
	const float mass = 80.0;
	btVector3 localInertia = btVector3(0, 0, 0);
	shape->calculateLocalInertia(mass, localInertia);
	glm::vec3 pos = c->position;
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z))); // xyz of origin
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, shape, btVector3(0, 0, 0)); // To construct multiple rigit bodies with same construction info
	c->rigitBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(c->rigitBody);
}



void Bullet::step(const double deltaTime)
{
	// deltaTime * 1000 is around 0.18
	dynamicsWorld->stepSimulation(deltaTime * 1000, 1, 0.2f); // Params: deltaTime in seconds, maxSubStepSize, fixedTimeStep in seconds. dt < msss * fts must hold!
}

Bullet::~Bullet()
{
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)	//remove the rigidbodies from the dynamics world and delete them
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

	for (int j = 0; j<shapes.size(); j++)	//delete collision shapes
	{
		btCollisionShape* shape = shapes[j];
		shapes[j] = 0;
		delete shape;
	}
	
	delete dynamicsWorld;//delete dynamics world
	delete solver;//delete solver
	delete overlappingPairCache;//delete broadphase
	delete dispatcher;//delete dispatcher
	delete collisionConfiguration;
	shapes.clear();//next line is optional: it will be cleared by the destructor when the array goes out of scope
}

void Bullet::debugDraw() {
	dynamicsWorld->debugDrawWorld(); // Fill debugDrawer with bullet rendering data
	debugDrawer->draw(); // Draw the receivied rendering data
}

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