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
		dynamicsWorld->setGravity(btVector3(0, -1.0f, 0));
		
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
	ModelNode* mn = dynamic_cast<ModelNode*>(current);

	if (mn && mn->bounding && mn->meshes.size() > 0)
	{

		if (CONCURENT_THREADS_SUPPORTED == 0) // Hyperthreading unsupported, calculate normally
			distributeBoundingGeneration(mn);
		else  // Hyperthreaded bounding volume generation
		{
			if (threads->size() < CONCURENT_THREADS_SUPPORTED) // If the system thread maximum is not reached, create a thread and calc bounding volume
				threads->push_back(async(launch::async, &Bullet::distributeBoundingGeneration, this, mn));
			else // All threads busy, wait for one to finish and if so remove him
			{
				while (threads->size() >= CONCURENT_THREADS_SUPPORTED)
					removeFinished(threads);
				//At least one thread is removed, so start for this one
				threads->push_back(async(launch::async, &Bullet::distributeBoundingGeneration, this, mn));
			}
		}
	}

	for (Node* child : current->children)
		createAndAddBoundingObjects(child);
}

void Bullet::join() 
{
	while (threads->size() > 0) // Wait until all remaining threads are finished and remove them
		removeFinished(threads);

	delete threads;
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

btConvexHullShape* Bullet::pureBulletConvexHullGeneration(ModelNode* mn)
{
	btConvexHullShape* btShape = new btConvexHullShape();//This constructor does not work (btScalar*)vhacdConvexHull.m_points, vhacdConvexHull.m_nPoints / 3, 3 * sizeof(double));
	vector<float>* vertices = mn->getAllVertices();

	for (int i = 0; i < vertices->size();)
	{
		btVector3 point;
		point.setX(vertices->at(i++));
		point.setY(vertices->at(i++));
		point.setZ(vertices->at(i++));
		btShape->addPoint(point, false);
	}

	btShape->optimizeConvexHull();
	btShape->recalcLocalAabb();
	vertices->clear();
	delete vertices;

	return btShape; // Max 100 vertices
}

bool Bullet::distributeBoundingGeneration(ModelNode* mn)
{
	if (mn->name.find(ModelLoader::getInstance()->IMMOVABLE_SUFFIX) != string::npos)
		createBuilding(mn);
	else
	{
		//BVG* bvg = new BVG(); // VHACD calculation
		btConvexHullShape* shape = pureBulletConvexHullGeneration(mn); // bvg->nodeCalculation(mn); // Buggy, false parameters? Max 100 vertices
		//delete bvg;
		
		shape->initializePolyhedralFeatures(); // Changing the collision shape now bad idea,  That will make the debug rendering more pretty, but doesn't change anything related to collision detection etc. http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=11385&p=38354&hilit=initializePolyhedralFeatures#p38354
		const float mass = 5.0;
		btVector3 localInertia = btVector3(0, 0, 0);
		shape->calculateLocalInertia(mass, localInertia);
		shape->setMargin(DEFAULT_COLLISION_MARGIN);
		
		btTransform trans;
		trans.setIdentity();
		trans.setFromOpenGLMatrix((btScalar*)&mn->hirachicalModelMatrix);  
	
		btDefaultMotionState* myMotionState = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo ci(mass, myMotionState, shape, localInertia);
		btRigidBody* rB = new btRigidBody(ci);
		mn->rigidBody = rB;
		shapes.push_back(shape);
		dynamicsWorld->addRigidBody(rB);
	}

	return true;
}

void Bullet::createBuilding(ModelNode* mn)
{
	btTriangleIndexVertexArray* meshArray = new btTriangleIndexVertexArray();
	//BVG* bvg = new BVG(); // VHACD calculation causes a bullet exception which should net happen according to their own comments

	for (unsigned int meshIndex = 0, STRIDE = 3; meshIndex < mn->meshes.size(); meshIndex++) // For vertex indices and vertices, see BVG.cpp for explanation is the stride
	{
		Mesh* glMesh = mn->meshes.at(meshIndex);
		btIndexedMesh btMesh;

		btMesh.m_indexType = PHY_INTEGER;
		btMesh.m_numTriangles = glMesh->indices.size() / STRIDE;
		btMesh.m_triangleIndexStride = STRIDE * sizeof(unsigned int);
		btMesh.m_triangleIndexBase = (const unsigned char*)glMesh->indices.data();// Allocate memory for the mesh

		vector<float>* vertices = glMesh->getAllVertices();

		btMesh.m_vertexType = PHY_FLOAT;
		btMesh.m_numVertices = vertices->size() / STRIDE;
		btMesh.m_vertexStride = STRIDE * sizeof(float);
		btMesh.m_vertexBase = (const unsigned char*)vertices->data();// Allocate memory for the mesh
		meshArray->addIndexedMesh(btMesh);
	}

	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(meshArray, true, true); // A single mesh with all vertices of a big object in it confuses bullet and generateds an "overflow in AABB..." error
	shape->buildOptimizedBvh();
	shape->setMargin(DEFAULT_COLLISION_MARGIN);
	mn->collisionObject = new btCollisionObject(); // Use btCollisionObject since a btRigitBody is just a subclass with mass and inertia which is not needed here
	mn->collisionObject->setCollisionShape(shape);

	btTransform trans;
	trans.setIdentity();
	trans.setFromOpenGLMatrix((btScalar*) &mn->hirachicalModelMatrix); //InverseHirachical causes perfectly skinned, but only for 1 building-colission mesh (last)
	mn->collisionObject->setWorldTransform(trans);
			
	shapes.push_back(shape);
	dynamicsWorld->addCollisionObject(mn->collisionObject);
}

void Bullet::createCamera(Camera* c) 
{
	btCylinderShape* shape= new btCylinderShape(btVector3(0.7f, 1.4f, 0.2f));
	shape->setMargin(DEFAULT_COLLISION_MARGIN);
	const float mass = 10.0;
	btVector3 localInertia = btVector3(0, 0, 0);
	shape->calculateLocalInertia(mass, localInertia);

	mat4 matrix = mat4();
	matrix[3] = vec4(c->position, 1.0f);

	btTransform trans;
	trans.setIdentity();
	trans.setFromOpenGLMatrix((btScalar*)&matrix);


	btDefaultMotionState* groundMotionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, groundMotionState, shape, localInertia); // To construct multiple rigit bodies with same construction info
	c->rigitBody = new btRigidBody(groundRigidBodyCI);
	shapes.push_back(shape);
	dynamicsWorld->addRigidBody(c->rigitBody);
}

btDiscreteDynamicsWorld* Bullet::getDynamicsWorld()
{
	
	return dynamicsWorld;
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