#include "Bullet.hpp"
#include "BVG.hpp"
#include "../ModelLoader.hpp"
#include "../../Camera/Camera.hpp"
#include "../../Camera/Frustum.hpp"
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
		dynamicsWorld->setGravity(btVector3(0, -8.0f, 0)); //earthgravity //10 is too high
		
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
	if(mn->name.find("_hinge") != string::npos)
		createDoorHinge(mn);
	else if (mn->name.find("MilitaryWoman") != string::npos)
		createEnemy(mn);
	else if (mn->name.find(ModelLoader::getInstance()->IMMOVABLE_SUFFIX) != string::npos)
		createBuilding(mn);
	else
	{
		//BVG* bvg = new BVG(); // VHACD calculation
		btConvexHullShape* shape = pureBulletConvexHullGeneration(mn); // bvg->nodeCalculation(mn);
		//delete bvg;
		
		shape->initializePolyhedralFeatures(); // Changing the collision shape now bad idea,  That will make the debug rendering more pretty, but doesn't change anything related to collision detection etc. http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=11385&p=38354&hilit=initializePolyhedralFeatures#p38354
		float mass = 5.0f;

		if (mn->name.find(ModelLoader::getInstance()->TABLE_NAME) != string::npos)
			mass = 40.0f;

		btVector3 localInertia = btVector3(0, 0, 0);
		shape->calculateLocalInertia(mass, localInertia);
		shape->setMargin(DEFAULT_COLLISION_MARGIN);
		
		btTransform trans;
		trans.setIdentity();
		removeScaleMatrix(mn->hirachicalModelMatrix, shape, &trans);
	
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
	removeScaleMatrix(mn->hirachicalModelMatrix, mn->collisionObject->getCollisionShape(), &trans);

	mn->collisionObject->setWorldTransform(trans);
	shapes.push_back(shape);
	dynamicsWorld->addCollisionObject(mn->collisionObject);
}

void Bullet::createDoorHinge(ModelNode* mn) 
{
	ModelNode* parentAngle = dynamic_cast<ModelNode*>(mn->parent);
	btConvexHullShape* shape = pureBulletConvexHullGeneration(mn); //this brings something ugly and "destroyed"
	shape->setLocalScaling(btVector3(0.8f, 0.8f, 0.8f));  //we cannot use a collision as big as the door itself, due to collision of surrounding building
 
	const float mass = 40.0f;
	btVector3 localInertia = btVector3(1, 1, 1); //setting localInertia to 0,0,0 breaks the program!
	shape->calculateLocalInertia(mass, localInertia);
	shape->setMargin(DEFAULT_COLLISION_MARGIN);

	btTransform trans;
	trans.setIdentity();
	removeScaleMatrix(mn->hirachicalModelMatrix, shape, &trans);
	shape->setLocalScaling(btVector3(0.8f, 0.8f, 0.8f));  //scale has to be set after removeScaleMatrix<--!, otherwise Problems of resolving collisions!

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, groundMotionState, shape, localInertia); // To construct multiple rigit bodies with same construction info
	btRigidBody *mydoor = new btRigidBody(groundRigidBodyCI);
	mn->rigidBody = mydoor;
	
	if(mn->name.find("lockedDoor") != string::npos) //set the locked door to "locked"
		mydoor->setAngularFactor(btVector3(0, 0, 0));

	 //values may only be positive here
	mydoor->setLinearFactor(btVector3(1, 0, 1)); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	mydoor->setAngularFactor(btVector3(0, 1, 0)); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	mydoor->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f)); // should be 0 0 0, https://en.wikipedia.org/wiki/Angular_velocity
	//c->rigitBody->setLinearVelocity() // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_cap_the_speed_of_my_spaceship
	//c->rigitBody->setAnisotropicFriction(btVector3(0.1f, 0.1f, 0.1f)); // https://docs.blender.org/api/intranet/docs/develop/physics-faq.html#What is Anisotropic Friction?
	mydoor->setFriction(0.0f);
	mydoor->setDamping(0.7f, 0.7f); //sets linear damping + angular damping
	mydoor->setMassProps(mass, localInertia);
	mydoor->setRestitution(0.0f); // 0 for switch off bouncing 
	shapes.push_back(shape);
	dynamicsWorld->addRigidBody(mydoor);

	btVector3 mnposition = btVector3(mn->hirachicalModelMatrix[3].x, mn->hirachicalModelMatrix[3].y, mn->hirachicalModelMatrix[3].z);
	btVector3 parentPosition = btVector3(parentAngle->hirachicalModelMatrix[3].x, parentAngle->hirachicalModelMatrix[3].y, parentAngle->hirachicalModelMatrix[3].z);
	btVector3 distanceDoorToHinge = btVector3(parentAngle->hirachicalModelMatrix[3].x - mn->hirachicalModelMatrix[3].x,
								   parentAngle->hirachicalModelMatrix[3].y - mn->hirachicalModelMatrix[3].y,
								   parentAngle->hirachicalModelMatrix[3].z - mn->hirachicalModelMatrix[3].z);

	btHingeConstraint* hingeDoorConstraint = new btHingeConstraint(*mydoor, distanceDoorToHinge, btVector3(0, 1, 0), true); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Constraints#Hinge_rotation_limits
	float lowRadians = 0.0f;
	float highRadians = Frustum::getInstance()->degreesToRadians(90.0f);

	if (mn->name.find("_reverse") != string::npos) //Reverse door limits for the ones which open in the other direction
	{
		float temp = lowRadians;
		lowRadians = -highRadians;
		highRadians = temp;
	}

	hingeDoorConstraint->setLimit(lowRadians, highRadians, 1.0f, 0.3f, 1.0f); // low, high, softness, biasFactor, relaxationFactor
	dynamicsWorld->addConstraint(hingeDoorConstraint);
}

void Bullet::createCamera(Camera* c) 
{
	btCylinderShape* shape= new btCylinderShape(btVector3(0.7f, 2.1f, 0.2f));
	shape->setMargin(DEFAULT_COLLISION_MARGIN);
	const float mass = 5.0;
	btVector3 localInertia = btVector3(1.0, 1.0, 1.0);
	shape->calculateLocalInertia(mass, localInertia);

	mat4 matrix = mat4();
	matrix[3] = vec4(c->position, 1.0f);

	btTransform trans;
	trans.setIdentity();
	removeScaleMatrix(matrix, shape, &trans);
	//trans.setOrigin(btVector3(0.0f, 2.0f, 0.0f)); //set the camera to the "head" does not affect kamera? just moves the position, nothing else

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, groundMotionState, shape, localInertia); // To construct multiple rigit bodies with same construction info
	c->rigitBody = new btRigidBody(groundRigidBodyCI);
	// we want a turn only on y-Axis
	c->rigitBody->setAngularFactor(btVector3(0, 1, 0)); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	// and movement only x-z (normally)
	//but we need a 1 in y-Axis for the LinearFactor, otherwise Collision-Detection gets nullified in this Axis
	c->rigitBody->setLinearFactor(btVector3(1, 1, 1)); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	//angular velocity should be 
	c->rigitBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f)); // https://en.wikipedia.org/wiki/Angular_velocity
	//c->rigitBody->setLinearVelocity() // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_cap_the_speed_of_my_spaceship
	//c->rigitBody->setAnisotropicFriction(btVector3(0.1f, 0.1f, 0.1f)); // https://docs.blender.org/api/intranet/docs/develop/physics-faq.html#What is Anisotropic Friction?
	c->rigitBody->setFriction(btScalar(0.8f));
	c->rigitBody->setDamping(btScalar(0.1f), btScalar(0.25f)); //sets linear damping + angular damping
	c->rigitBody->setRestitution(btScalar(0.1f)); //little bounce on the body
	c->rigitBody->setSleepingThresholds(btScalar(0.2f), btScalar(0.2f)); // linear, angular 
	//btVector3 inertia;
	//c->rigitBody->getCollisionShape()->calculateLocalInertia(mass, inertia);
	//c->rigitBody->setMassProps(mass, localInertia); //unnecessary?
	//c->rigitBody->setAngularFactor(btVector3(0, 0, 0));
	/*
	You just need to call btRigidBody::setAngularFactor(btVector3(Yaw, Pitch, Roll)); Calling it with all 0s will prevent your object from rotating on any angle.
	*c->rigitBody->setAngularDamping = 0.5f;
	c->rigitBody->setLinearDamping = 0.5f;
	c->rigitBody.m_linearSleepingThreshold = 0.2f;
	c->rigitBody.m_angularSleepingThreshold = 0.2f;
	c->rigitBody.m_restitution = 0.5f;
	*/
	shapes.push_back(shape);
	dynamicsWorld->addRigidBody(c->rigitBody);
}

void Bullet::createEnemy(ModelNode* mn)
{
	btCylinderShape* shape = new btCylinderShape(btVector3(0.7f, 0.05f, 0.2f));
	shape->setMargin(DEFAULT_COLLISION_MARGIN);
	const float mass = 5.0;
	btVector3 localInertia = btVector3(1.0, 1.0, 1.0);
	shape->calculateLocalInertia(mass, localInertia);

	btTransform trans;
	trans.setIdentity();
	removeScaleMatrix(mn->hirachicalModelMatrix, shape, &trans);

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, groundMotionState, shape, localInertia); // To construct multiple rigit bodies with same construction info
	btRigidBody *myEnemy = new btRigidBody(groundRigidBodyCI);
	//mydoor->
	mn->rigidBody = myEnemy;
	// we want a turn only on y-Axis
	mn->rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f)); //angular velocity should be https://en.wikipedia.org/wiki/Angular_velocity
	mn->rigidBody->setAngularFactor(btVector3(0, 1, 0)); // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	mn->rigidBody->setLinearFactor(btVector3(1, 1, 1)); // and movement only x-z (normally), but we need a 1 in y-Axis for the LinearFactor, otherwise Collision-Detection gets nullified in this Axis, http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_constrain_an_object_to_two_dimensional_movement.2C_skipping_one_of_the_cardinal_axes
	//c->rigitBody->setLinearVelocity() // http://bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_cap_the_speed_of_my_spaceship
	//c->rigitBody->setAnisotropicFriction(btVector3(0.1f, 0.1f, 0.1f)); // https://docs.blender.org/api/intranet/docs/develop/physics-faq.html#What is Anisotropic Friction?
	//mn->rigidBody->setFriction(btScalar(0.8f));
	//mn->rigidBody->setDamping(btScalar(0.1f), btScalar(0.25f)); //sets linear damping + angular damping
	//mn->rigidBody->setRestitution(btScalar(0.1f)); //little bounce on the body
	//mn->rigidBody->setSleepingThresholds(btScalar(0.2f), btScalar(0.2f)); // linear, angular 
	//mn->modelMatrix = glm::translate(mn->modelMatrix, glm::vec3(-5.4f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene															
	mn->rigidBody->activate(true);
	mn->isEnemy = true;
	//mn->rigidBody->translate(btVector3(-5.4f, 0, 0)); //put her into the floor
	mn->rigidBody->setActivationState(DISABLE_DEACTIVATION);
	shapes.push_back(shape);
	dynamicsWorld->addRigidBody(mn->rigidBody);
}

btDiscreteDynamicsWorld* Bullet::getDynamicsWorld()
{
	return dynamicsWorld;
}

Bullet::~Bullet()
{
	for (int i = dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)	//remove the contraints before the rigitbodys from the dynamics world and delete them
	{
		btTypedConstraint * constraint = dynamicsWorld->getConstraint(i);
		dynamicsWorld->removeConstraint(constraint);

		delete constraint;
	}

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

void Bullet::removeScaleMatrix(glm::mat4 matrix, btCollisionShape* shape, btTransform* trans)
{
	const float scaleX = glm::length(glm::vec3(matrix[0][0], matrix[0][1], matrix[0][2]));
	const float scaleY = glm::length(glm::vec3(matrix[1][0], matrix[1][1], matrix[1][2]));
	const float scaleZ = glm::length(glm::vec3(matrix[2][0], matrix[2][1], matrix[2][2]));
	glm::mat4 matWithoutScale = glm::scale(matrix, glm::vec3(1.0f / scaleX, 1.0f / scaleY, 1.0f / scaleZ));

	shape->setLocalScaling(btVector3(scaleX, scaleY, scaleZ));
	trans->setFromOpenGLMatrix(glm::value_ptr(matWithoutScale));
	//trans->setFromOpenGLMatrix(glm::value_ptr(matrix)); // Line to set original matrix
}
