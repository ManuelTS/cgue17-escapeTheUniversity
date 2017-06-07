#include "Bullet.hpp"
#include "BVG.hpp"
#include "../ModelLoader.hpp"
#include "../../Camera/Camera.hpp"
#include "../../Debug/Debugger.hpp"
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
			debugDrawer->setDebugMode(btIDebugDraw::DBG_FastWireframe);
			dynamicsWorld->setDebugDrawer(debugDrawer);
		#endif

		initialized = !initialized;
	}
}

void Bullet::createAndAddBoundingObjects(Node* current)
{
	if (concurentThreadsSupported == 0) // Hyperthreading unsupported, calculate normally
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);

		if (mn && mn->meshes.size() > 0 && mn->bounding)
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

			if (mn && mn->meshes.size() > 0 && mn->bounding)
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
	if (mn->name.find(ModelLoader::getInstance()->SMALL_WING) != string::npos || mn->name.find(ModelLoader::getInstance()->BIG_WING) != string::npos)
		createBuilding(mn);
	else
	{
		BVG* bvg = new BVG();
		btConvexHullShape* shape = bvg->calculateVHACD(mn); // Max 100 vertices
		delete bvg;
		shapes.push_back(shape);
		
		//shape->optimizeConvexHull();
		shape->initializePolyhedralFeatures(); // Changing the collision shape now bad idea,  That will make the debug rendering more pretty, but doesn't change anything related to collision detection etc. http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=11385&p=38354&hilit=initializePolyhedralFeatures#p38354
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

	return true;
}

void Bullet::createBuilding(ModelNode* mn)
{
	btTriangleIndexVertexArray* meshArray = new btTriangleIndexVertexArray();

	
	for (unsigned int meshIndex = 0; meshIndex < mn->meshes.size(); meshIndex++)
	{
		Mesh* mnMesh = mn->meshes.at(meshIndex);
		btIndexedMesh btMesh;
		const int stride = 3; // For vertex indices and vertices, see BVG.cpp for explanation

		// Source https://github.com/VDrift/vdrift/blob/master/src/trackloader.cpp#L75
		btMesh.m_indexType = PHY_INTEGER;
		btMesh.m_numTriangles = mnMesh->indices.size() / stride;
		btMesh.m_triangleIndexStride = stride * sizeof(unsigned int);
		btMesh.m_triangleIndexBase = (const unsigned char*) mnMesh->indices.data();// Allocate memory for the mesh
		
		vector<float>* vertices = mnMesh->getAllVertices();

		btMesh.m_vertexType = PHY_FLOAT;
		btMesh.m_numVertices = vertices->size() / stride;
		btMesh.m_vertexStride = stride * sizeof(float);
		btMesh.m_vertexBase = (const unsigned char*) vertices->data();// Allocate memory for the mesh

		meshArray->addIndexedMesh(btMesh);
			
		//delete vertices; // Must be active, bullet uses this pointer!
		Debugger::getInstance()->writeAllVertices(vertices, mn->name + "_init");
	}


	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(meshArray, true, true); // A single mesh with all vertices of a big object in it confuses bullet and generateds an "overflow in AABB..." error
	shapes.push_back(shape);
	//shape->buildOptimizedBvh();
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
	shapes.push_back(shape);
	shape->setMargin(0.05f);
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