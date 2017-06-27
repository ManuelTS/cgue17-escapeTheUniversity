#include "SoundManager.hpp"
#include "Camera/Frustum.hpp"
#include "RenderLoop.hpp"
#include "Model/ModelLoader.hpp"
#include "Model/Node/Node.hpp"
#include "Model/Node/LightNode.hpp"
#include "Model/Node/ModelNode.hpp"
#include "Model/Node/AnimatNode.hpp"
#include "Camera/Camera.hpp"
#include "Shader.hpp"
#include "Text.hpp"
#include "Model\Physic\Bullet.hpp"
#include "Debug/Debugger.hpp"
#include "Debug/MemoryLeakTracker.h"
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLM\gtc\type_ptr.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <GLFW/glfw3.h>
#include <windows.h>

using namespace std;

RenderLoop::~RenderLoop()
{
	delete initVar;
	delete camera;
	//delete window; window = nullptr; // Done by glTerminateWindow
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	RenderLoop* rl = RenderLoop::getInstance();

	if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_END) && action == GLFW_PRESS) // See Text.cpp#help for keybindings
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		rl->render = !rl->render;
		rl->help = !rl->help;
	}
	else if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
		rl->fps = !rl->fps;
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		rl->wireFrameMode = !rl->wireFrameMode;
	else if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
	{
		rl->textureSampling++;
		rl->changeQuality();
	}
	else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
	{
		rl->mipMapping++;
		rl->changeQuality();
	}
	else if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
		cout << "TODO: Visualizing the depth buffer." << endl; // TODO Visualizing the depth buffer http://learnopengl.com/#!Advanced-OpenGL/Depth-testing, swith shaders to depth ones
	else if ((key == GLFW_KEY_F7 || key == GLFW_KEY_PAUSE) && action == GLFW_PRESS)
		rl->render = !rl->render;
	else if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
	{
		rl->frustum = !rl->frustum;
		cout << "Switching Frustum Culling to ";
		if (rl->frustum)
			cout << "disabled." << endl;
		else
			cout << "enabled." << endl;
	}
	else if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
		rl->blending = !rl->blending;
	else if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
		rl->stencil = !rl->stencil;
	else if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
		rl->toggleFullscreen();
	//else if (key == GLFW_KEY_F12) // Causes an error
	else if (key == GLFW_KEY_SCROLL_LOCK && action == GLFW_PRESS)
		rl->drawBulletDebug = !rl->drawBulletDebug;
	else if ((key == GLFW_KEY_SLASH || key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_RIGHT_BRACKET || key == GLFW_KEY_KP_ADD) && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{ // slash is german minus and right bracket is german plus on a german keyboard
		bool minus = key == GLFW_KEY_SLASH || key == GLFW_KEY_KP_SUBTRACT; // In- or decrease ambient light coefficient

		for (LightNode* ln : ModelLoader::getInstance()->lights)
			ln->light.diffuse.a += minus ? -0.01f : 0.01f;
	}
	//else if (key == GLFW_KEY_E && action == GLFW_PRESS) is obsolete
	//	rl->move(ModelLoader::getInstance()->root);
	else if ((key == GLFW_KEY_Q || key == GLFW_KEY_E) && action == GLFW_PRESS) //pull or push
	{		
		Bullet* b = Bullet::getInstance();
		vec3 origin = vec3(rl->camera->position.x, rl->camera->position.y, rl->camera->position.z);
		vec3 distance = origin + rl->camera->front * 5.0f;
		//jep, this has to be done that way with the raytest!
		btCollisionWorld::ClosestRayResultCallback res(btVector3(origin.x, origin.y, origin.z), btVector3(distance.x, distance.y, distance.z));
		b->getDynamicsWorld()->rayTest(btVector3(origin.x, origin.y, origin.z), btVector3(distance.x, distance.y, distance.z), res);
		if (res.hasHit()) {
			printf("Raycast hit at: <%.2f, %.2f, %.2f>\n", res.m_hitPointWorld.getX(), res.m_hitPointWorld.getY(), res.m_hitPointWorld.getZ());
		}
		if (res.hasHit() && res.m_collisionObject->CO_RIGID_BODY) 
		{
			btRigidBody* body;
			body = (btRigidBody*)res.m_collisionObject;
			//btRigidBody* body = (btRigidBody*)btRigidBody::upcast(res.m_collisionObject); //this upcast does not work!!
			body->activate();
			//body->applyCentralForce(btVector3(4.0f, 0.0f,4.0f));

			if (rl->gamePhaseKey) {  //this "unlockes" the locked door
				body->setAngularFactor(btVector3(0, 1, 0));
			}
			
		
			if (key == GLFW_KEY_Q)                                    //push
			{
				//body->applyCentralForce(btVector3(-4.0f, 0.0f, -4.0f));
				body->applyTorqueImpulse(btVector3(0.0f, -4.0f, 0.0f)); //this is the way to go!
				//body->applyCentralImpulse(btVector3(-4.0f, 0.0f, -4.0f)); 
			}
			else 
			{
				body->applyTorqueImpulse(btVector3(0.0f, 4.0f, 0.0f));   //pull
				//body->applyCentralForce(btVector3(4.0f, 0.0f, 4.0f));
				//body->applyCentralImpulse(btVector3(4.0f, 0.0f, 4.0f));
			}
		}
	
	}	
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			rl->gameEventCheckIsOn = true; //we want to check only a single time for 1 frame, gets reseted afterwards	
		}
	else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) // ü on german keyboard, ü is for überflieger
		rl->freeCamera = !rl->freeCamera;

	else if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		rl->gameOver = false;  
		//Text::getInstance()->addText2Display(Text::GAME_OVER);
		//SoundManager::getInstance()->playSound("Dialog\\exmatriculated.mp3");
	}
	else if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		rl->disableKeyRendering = !rl->disableKeyRendering;
	}
	else if (key == GLFW_KEY_B && action == GLFW_PRESS)
		SoundManager::getInstance()->playSound("Dialog\\burp.mp3");
	else if (key == GLFW_KEY_PRINT_SCREEN && action == GLFW_PRESS)
		Text::getInstance()->addText2Display(Text::SCREENY);
	else if (key == GLFW_KEY_BACKSLASH && action == GLFW_PRESS) // # in german keyboard
		rl->showCamCoords = !rl->showCamCoords;
	else if (key == GLFW_KEY_MINUS && action == GLFW_PRESS) // ß in german keyboard
		rl->drawLightBoundingSpheres = !rl->drawLightBoundingSpheres;
	else if (key == GLFW_KEY_KP_ENTER && action == GLFW_PRESS) // Num enter on german keyboard
		rl->drawShadowMap = !rl->drawShadowMap;
	else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) // ü on german keyboard, ü is for überflieger
		rl->freeCamera = !rl->freeCamera;
}

/*Listens for user input.*/
void RenderLoop::doMovement(double timeDelta)
{
	RenderLoop* rl = RenderLoop::getInstance();
	btTransform trans = camera->rigitBody->getCenterOfMassTransform();
	mat4 matrix;

	if (!freeCamera) // Constrains camera movement to bullet physics
	{ //Set camera postion after the physics from the last frame were calculated
		trans.getOpenGLMatrix(glm::value_ptr(matrix));
		camera->position = vec3(matrix[3]); // only update the position of the camera!
	}

	float factor = 0.8f;
	vec3 movementVectorXYAxis = vec3(1.0f * factor, 1.0f * factor, 1.0f *factor); //direction of possible axis * factor  (otherwise fast)	


	//Camera controls
    //jitter cannot be solved through applying the force just on key-tap 
	//jitter cannot be solved through applyCentralForce on body (though, keep in mind, force needs to be adjusted to the mass)
	// setInterpolationLinearVelocity could maybe work, needs investigation
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
	{
		if(freeCamera)
			camera->processKeyboard(camera->FORWARD, timeDelta);
		//camera->rigitBody->setLinearVelocity(btVector3(movement.x, movement.y, movement.z));
		//camera->rigitBody->applyCentralForce(
		camera->rigitBody->setActivationState(true);
		vec3 movement = camera->front*movementVectorXYAxis;
		camera->rigitBody->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z));
		rl->lastImpulse = vec3(-movement.x, -movement.y, -movement.z);
		//int i = camera->rigitBody->getWorldArrayIndex(); //this works for identification of the object	
		//camera->rigitBody->applyCentralForce(btVector3(movement.x, movement.y, movement.z));
	}	
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (freeCamera) 
				camera->processKeyboard(camera->BACKWARD, timeDelta);
		camera->rigitBody->setActivationState(true);
		vec3 movement = camera->front*(-movementVectorXYAxis); //we want to walk backwards
		camera->rigitBody->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z));
		rl->lastImpulse = vec3(-movement.x, -movement.y, -movement.z);
	//	printf("Detection down: <%.2f>\n", camera->rigitBody->getWorldTransform());
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (freeCamera)
			camera->processKeyboard(camera->LEFT, timeDelta); 
		camera->rigitBody->setActivationState(true);
		vec3 movement = glm::normalize(glm::cross(camera->front, camera->up))*(-movementVectorXYAxis); //going left!
		camera->rigitBody->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z));
		rl->lastImpulse = vec3(-movement.x, -movement.y, -movement.z);
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (freeCamera)
			camera->processKeyboard(camera->RIGHT, timeDelta); 
		camera->rigitBody->setActivationState(true);
		vec3 movement = glm::normalize(glm::cross(camera->front, camera->up))*(movementVectorXYAxis); //going Right!
		camera->rigitBody->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z));
		rl->lastImpulse = vec3(-movement.x, -movement.y, -movement.z);
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) //make jump forward
	{
		if (freeCamera)
			camera->processKeyboard(camera->UP, timeDelta);
		camera->rigitBody->setActivationState(true);
		//
		//camera->rigitBody->applyCentralImpulse(btVector3(0, 80.0, 0));//, btVector3(camera->position.x, camera->position.y, camera->position.z));
		//neither applyForce, applyImpulse, work
		vec3 movement = camera->front*movementVectorXYAxis;
		//camera->rigitBody->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z));
		camera->rigitBody->applyCentralImpulse(btVector3(movement.x, 3.0f, movement.z));
		//printf("Detection Space: <%.2f>\n", camera->rigitBody->getFlags());
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		//camera->processKeyboard(camera->UP, timeDelta);
		camera->rigitBody->setActivationState(true);
	//	camera->rigitBody->setLinearVelocity(btVector3(0, -3, 0));
		camera->rigitBody->applyCentralImpulse(btVector3(0, -3.0f, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && 
		glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE &&
		glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE &&
		glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
	{
		//vec3 movement = camera->front*movementVectorXYAxis;
		//camera->rigitBody->setLinearVelocity(btVector3(movement.x, movement.y, movement.z));
		camera->rigitBody->setActivationState(true);
		camera->rigitBody->setLinearVelocity(btVector3(0, 0, 0));
		//camera->rigitBody->applyCentralImpulse(btVector3(rl->lastImpulse.x*2, rl->lastImpulse.y*2, rl->lastImpulse.z*2));
		camera->rigitBody->applyCentralImpulse(btVector3(0, -5, 0));
		rl->lastImpulse = vec3(0, 0.0f, 0);
	}
	else { // if the key of movement stops, we should put the velocity to 0 (otherwise it will continue to move)
		camera->rigitBody->setActivationState(true);
		//camera->rigitBody->setLinearVelocity(btVector3(0, 0, 0));
		//activationstate shall be be automatically set to false if no force is applied
	}
	if (!freeCamera)
	{
		matrix = mat4(); // No yaw or pitch
		matrix[3] = vec4(camera->position, 1.0f); // Set the new position to the bounding camera object
		trans.setFromOpenGLMatrix(glm::value_ptr(matrix));
		//camera->rigitBody->getMotionState()->setWorldTransform(trans);
		
		/*
		first: 
		https://gamedev.stackexchange.com/questions/24772/how-would-i-move-a-character-in-an-rpg-with-bullet-physics-ogre3d

		apply linear velocity

		*/

		/*
		Keep in mind:
		Perhaps the object becomes deactivated and then it doesn't move anymore. Then you have to activate it again
		(search in the documentation how to activate a rigid body with the setActivationState() method)
		Objects become deactivated when they become almost stopped. 
		Then they don't activate again until another object applies them a force or you do it programmatically.
		*/

		/*if above did not work:
		https://stackoverflow.com/questions/12251199/re-positioning-a-rigid-body-in-bullet-physics
		I can't tell you what causes the unusual outcome when moving rigid bodies but I can definitely sympathize!

		There are three things you'll need to do in order to solve this:

		Convert your rigid bodies to kinematic ones
		Adjust the World Transform of the bodies motion state and NOT the rigid body
		Convert the kinematic body back to a rigid body

		And/or
		Change its worldtransform and then clear forces and linear and angular velocities is sufficient
		*/

	}
}

void RenderLoop::move(Node* current)
{
	TransformationNode* dn = dynamic_cast<TransformationNode*>(current); 
	
	if (dn && (dn->name.find(ModelLoader::getInstance()->ANGLE_SUFFIX) != string::npos)) {// != string::npos && Frustum::getInstance()->sphereInFrustum(vec3(dn->position), 2) >-1)//has to be TransformationNode, only open doors "_angle" not animation nodes
		 //get the according door
		for (Node*child : current->children) {
			ModelNode* mn = dynamic_cast<ModelNode*>(child);
			if (mn) {
				if (Frustum::getInstance()->inActionRadius(vec3(mn->hirachicalModelMatrix[3])) == 1) // here we use the hirachicalModelMatrix of the according door for interaction
					//	if(Frustum::getInstance()->sphereInFrustum(vec3(dn->hirachicalModelMatrix[3]), 2.5) >-1)
					dn->switchState();
			}	
		} //boy, that's ugly but working
	}

	for (Node*child : current->children)
		move(child);
}

/*Callback function for windowresize, set viewport to the entire window.
* todo Adjust projection matrix and Perspective*/
void resizeCallback(GLFWwindow *wd, int width, int height) {
	// Set the viewport to be the entire window
	RenderLoop* rl = RenderLoop::getInstance();
	glViewport(0, 0, rl->width = width, rl->height = height);

	Frustum::getInstance()->setCamInternals(rl->initVar->zoom, width, height);
}

void scrollCallback(GLFWwindow* window, double xpos, double ypos) // this method is specified as glfw callback
{
	//here we access the instance via the singleton pattern and forward the callback to the instance method
	RenderLoop* instance = RenderLoop::getInstance();
	instance->xScroll += xpos;
	instance->yScroll += ypos;

	if (instance->xScroll < 0)
		instance->xScroll = 0;

	if (instance->yScroll < 0)
		instance->yScroll = 0;

	//instance->camera->processMouseScroll(instance->yScroll); // TODO the zoom?
}

void mouseCallback(GLFWwindow* window, double x, double y)
{
	RenderLoop::getInstance()->camera->processMouseMovement(x, y);
}

void errorCallback(int error, const char* description)
{
	Debugger::getInstance()->errorCallback(error, description);
}

// Initializes GLFW and GLEW
void RenderLoop::initGLFWandGLEW() {
	if (!glfwInit())
		Debugger::getInstance()->pauseExit("Could not init GLFW.");

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
	#if _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Using OpenGL version 4.3, 4.4 could be used if necessary
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, initVar->fps);

	window = glfwCreateWindow(initVar->width, initVar->height, initVar->windowTitle, (fullscreen = initVar->fullscreen) ? glfwGetPrimaryMonitor() : nullptr, nullptr);

	if (!window)
		Debugger::getInstance()->pauseExit("Window initialization failed.");

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	GLenum err = glewInit();

	glGetError(); // Error 1280 after glewInit() internet says this one call can be ignored

	#if _DEBUG
		InitMemoryTracker();
		Debugger* d = Debugger::getInstance();
		d->setDebugContext();
		glfwSetErrorCallback(errorCallback);
	#endif

	glfwSetKeyCallback(window, keyCallback);// Set callbacks
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetWindowSizeCallback(window, resizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);

	if (err != GLEW_OK)
	{
		cerr << glewGetErrorString(err) << endl;
		glfwTerminate();
		Debugger::getInstance()->pauseExit("Failed to initialize GLEW.");
	}

	if (!glewIsSupported("GL_VERSION_4_3"))
		Debugger::getInstance()->pauseExit("OpenGL 4.3 is needed for this game, you cannot continue but there is no guarantee that it will work properly."); // TODO Display on screen

	Frustum::getInstance()->setCamInternals(initVar->zoom, width, height);
}

void RenderLoop::start()
{ // Init all
	time.past = glfwGetTime();
	ModelLoader* ml = ModelLoader::getInstance();
	SoundManager* sm = SoundManager::getInstance();
	sm->init();
	#if !_DEBUG
		sm->setFileName("Music\\Jahzzar_-_01_-_The_last_ones.mp3"); // Init SM with music file to play while loading
		sm->playSound();
	#endif

	initVar = new Initialization();

	width = initVar->width;
	height = initVar->height;
	glViewport(0, 0, width, height);

	camera = new Camera(glm::vec3(6.2f,16.4,0.4f), initVar->zoom, initVar->movingSpeed, initVar->mouseSensitivity);

	initGLFWandGLEW();
	displayLoadingScreen(ml);

	ml->load("Playground.dae"); // Load Models

	Bullet* b = Bullet::getInstance(); // Calculate bouding volumes, no pointer deletion since it is a singelton!
	b->init();
	b->createAndAddBoundingObjects(ml->root); // Sets pointers of rigitBodies in all nodes of the scene graph
	b->join();
	b->createCamera(camera); // Create cam bounding cylinder

	//glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

	GBuffer* gBuffer = new GBuffer(initVar->maxWidth, initVar->maxHeight);
	ShadowMapping* realmOfShadows = new ShadowMapping();

	sm->stopAll(); // Stop loading sound
	while (!glfwWindowShouldClose(window)) // Start rendering
	{
		calculateDeltaTime();
		glfwPollEvents(); // Check and call events
		doMovement(time.delta);

		if (render) // normal game rendering
		{
			doDeferredShading(gBuffer, realmOfShadows, ml);
			gameEventCheckIsOn = false; //we reset the actions for key/paper/end-zone every frame
		}
		else
		{
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to black
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		renderText();
		glfwGetWindowSize(window, &width, &height);
		glfwSwapBuffers(window);
	}

	#if !_DEBUG
		sm->playSound("Dialog\\Bye.mp3"); // Exit sound
		Sleep(1600);
	#endif
	// TODO: Write with initVar game statistics, play time etc...
	delete realmOfShadows;
	delete gBuffer;
	glfwDestroyWindow(window);
	glfwTerminate();
}

void RenderLoop::doDeferredShading(GBuffer* gBuffer, ShadowMapping* realmOfShadows, ModelLoader* ml)
{
	if (wireFrameMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (fps)
		drawnTriangles = 0;

	Frustum* frustumCulling = Frustum::getInstance();
	Shader* gBufferShader = gBuffer->gBufferShader;

	// Deferred Shading: Geometry Pass, put scene's gemoetry/color data into gbuffer
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE); // Must be before glClearColor and ShadowMapping, otherwise it remains untouched
	glDepthFunc(GL_LEQUAL);
	gBuffer->clearFrame();
	gBuffer->bindForGeometryPass(); // Previously the FBO in the G Buffer was static (in terms of its configuration) and was set up in advance so we just had to bind it for writing when the geometry pass started. Now we keep changing the FBO to we need to config the draw buffers for the attributes each time.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear everything inside the buffer for new clean, fresh iteration
	gBufferShader->useProgram();
	const float const* projectionMatrixP = glm::value_ptr(glm::perspective(frustumCulling->degreesToRadians(camera->zoom), (float)width / (float)height, frustumCulling->nearD, frustumCulling->farD));
	const float const* viewMatrixP = glm::value_ptr(camera->getViewMatrix());
	glUniformMatrix4fv(gBufferShader->projectionLocation, 1, GL_FALSE, projectionMatrixP);
	glUniformMatrix4fv(gBufferShader->viewLocation, 1, GL_FALSE, viewMatrixP);
	glUniform4f(ModelNode::debugFlagLocation, 0, 0, 0, 0); // x = flag for debugging to render bullet wireframe with value 1
	draw(ml->root); // Draw all nodes except light ones

	if (drawBulletDebug) // Draws the bullet debug context, see bullet.cpp#bulle
		Bullet::getInstance()->debugDraw();

	// Stencil and light passes
	Shader* deferredShader = gBuffer->deferredShader;
	for (unsigned int i = 0; i < ml->lights.size(); i++) // Look which lights intersect or are in the frustum
	{
		LightNode* ln = ml->lights.at(i);
		ln->light.position.w = gBuffer->calcPointLightBSphere(ln); // Calculate the light sphere radius
		bool lightInFrustum = frustumCulling->sphereInFrustum(vec3(ln->light.position), ln->light.position.w) > -1;// See lightNode.hpp, use the radius of the light volume to cull lights not inside the frustum

		if (drawLightBoundingSpheres) // Draws the light bounding sphere of all lights
		{
			gBuffer->bindForGeometryPass();
			gBufferShader->useProgram();
			Debugger::getInstance()->drawLightBoundingSpheres(ln); // TODO buggy
		}

		if (frustum || lightInFrustum)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			realmOfShadows->renderInDepthMap(ml->root, ln, initVar->zoom, width, height); // far plane is the spheres radius

			if (drawShadowMap)
				Debugger::getInstance()->renderShadowMap(ln->light.position.w, realmOfShadows->dephMapTextureHandle); // TODO impossible since the foor loop and directional lights afterwards
			else
			{
				// Stencil
				gBuffer->bind4StencilPass(); // Return from shadow FBO to light FBO
				gBuffer->stencilShader->useProgram();
				glEnable(GL_STENCIL_TEST);
				glStencilMask(0xFF);
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS, 0, 0x00);
				glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
				glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
				glDepthMask(GL_FALSE); // No depth writing, only reading
				glDisable(GL_CULL_FACE);

				glm::mat4 sphereModelMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(ln->light.position)), glm::vec3(ln->light.position.w)); // Transalte then scale sphere model matrix
				const float const* sphereModelMatrixP = glm::value_ptr(ml->sphere01->hirachicalModelMatrix = sphereModelMatrix);
				glUniformMatrix4fv(ModelNode::modelLocation, 1, GL_FALSE, sphereModelMatrixP);
				glUniformMatrix4fv(ModelNode::viewMatrixStencilLocation, 1, GL_FALSE, viewMatrixP); // stencil.vert
				glUniformMatrix4fv(ModelNode::projectionMatrixStencilLocation, 1, GL_FALSE, projectionMatrixP); // stencil.vert

				for (Mesh* m : ml->sphere01->meshes)
					m->draw(GL_TRIANGLES, true);

				glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Only write values that have a non zero stencil value
				glStencilMask(0x00); // Writing disallowed, reading allowed
				glDisable(GL_DEPTH_TEST);

				//Light pass
				gBuffer->bind4LightPass();
				deferredShader->useProgram();
				gBuffer->bindTextures();
				realmOfShadows->bindTexture(); 	//Write shadow data to deferredShader.frag. Link depth map into deferred Shader fragment

				if (blending)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glBlendEquation(GL_FUNC_ADD);
				}

				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);

				glUniform3fv(deferredShader->viewPositionLocation, 1, &camera->position[0]);// Write light data to deferred shader.frag
				glUniformMatrix4fv(realmOfShadows->SHADOW_LIGHT_SPACE_MATRIX_LOCATION, 1, GL_FALSE, glm::value_ptr(realmOfShadows->lightSpaceMatrix)); // Write the light space matrix to the deferred shader
				glBindBufferBase(GL_UNIFORM_BUFFER, ml->lightBinding, ml->lightUBO); // OGLB: S. 169, always execute after new program is used
				glBindBuffer(GL_UNIFORM_BUFFER, ml->lightUBO);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ln->light), &ln->light);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				// Render the light sphere positions through the deferred shading shader.vert and blend them together with already existing result in attachment 2
				glUniformMatrix4fv(ModelNode::modelLocation, 1, GL_FALSE, sphereModelMatrixP); // deferredShading.vert
				glUniformMatrix4fv(ModelNode::viewMatrixStencilLocation, 1, GL_FALSE, viewMatrixP); // deferredShading.vert
				glUniformMatrix4fv(ModelNode::projectionMatrixStencilLocation, 1, GL_FALSE, projectionMatrixP); // deferredShading.vert

				for (Mesh* m : ml->sphere01->meshes)
					m->draw(GL_TRIANGLES, true);

				glCullFace(GL_BACK);
				if (blending)
				{
					//glDisable(GL_ALPHA_TEST); //Depricated in openGL 4.3
					glDisable(GL_BLEND);
				}
				glDisable(GL_STENCIL_TEST);
				gBuffer->unbindTexture();
				realmOfShadows->unbindTexture();
			}
			glDisable(GL_DEPTH_TEST); 
		}
	}

	// Directional light pass, it light does not need a stencil or depth test because its volume is unlimited and the final pass simply copies the texture, in our case this is obly the ambient light
	gBuffer->bind4LightPass(); // Return from shadow FBO to light FBO
	deferredShader->useProgram();
	gBuffer->bindTextures();
	realmOfShadows->bindTexture(); // Write shadow data to deferredShader.frag. Link depth map into deferred Shader fragment
	glUniform3fv(deferredShader->viewPositionLocation, 1, &camera->position[0]);// Write light data to deferred shader.frag
	glBindBufferBase(GL_UNIFORM_BUFFER, ml->lightBinding, ml->lightUBO); // OGLB: S. 169, always execute after new program is used
	glBindBuffer(GL_UNIFORM_BUFFER, ml->lightUBO);
	LightNode::Light ambientLight = LightNode::Light();
	ambientLight.diffuse = vec4(1, 0, 0, ambientLight.diffuse.a);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ambientLight), &ambientLight);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	const float* unitMatrixP = glm::value_ptr(glm::mat4()); // No cam perspective or view needed because the whole gbuffer is rendered
	glUniformMatrix4fv(ModelNode::modelLocation, 1, GL_FALSE, unitMatrixP); // deferredShading.vert
	glUniformMatrix4fv(ModelNode::viewMatrixStencilLocation, 1, GL_FALSE, unitMatrixP); // deferredShading.vert
	glUniformMatrix4fv(ModelNode::projectionMatrixStencilLocation, 1, GL_FALSE, unitMatrixP); // deferredShading.vert

	if (blending)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	gBuffer->drawDirectionalLight();

	if (blending)
		glDisable(GL_BLEND);

	gBuffer->unbindTexture();
	realmOfShadows->unbindTexture();

	if (wireFrameMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	gBuffer->finalPass(width, height);// Final pass, blit attachment2 to standard FBO (screen)
}

void RenderLoop::draw(Node* current)
{
	
	if (current && !dynamic_cast<LightNode*>(current)) // Don't draw light nodes
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);
/*
		GAME LOGIC WINNING CONDITION		
*/
		//doing the checks the other way around, so messages dont overlap
		if (gameEventCheckIsOn && gamePhasePaper && gamePhaseKey &&  string::npos != mn->name.find("WinningZone"))
		{
			checkGamePhaseEnd(mn);
		}
		else if (gamePhaseKey && gameEventCheckIsOn  && string::npos != mn->name.find("Paperstack"))
		{
			checkGamePhasePaper(mn);
		}
		else if (gameEventCheckIsOn && string::npos != mn->name.find("Key"))
		{	
			checkGamePhaseKey(mn);			
		}
		
		/* do check if in enemy sight for GAMEOver!*/
		if (mn && mn->isEnemy && !gameOver)
		{
			checkGameOverCondition(mn);			
		}


// continue normal renderloop
		
		if (mn) // Leave if structure this way!
		{
			// If model node and (no frustum or transformationNode or modelNode bounding frustum sphere (modelNode center, radius) inside frustum):
			// ... render only when the model node schould be rendered

			if (mn->name.find("Scene") != string::npos || 
				(mn->render && 
					(frustum || 
					 Frustum::getInstance()->sphereInFrustum(vec3(mn->hirachicalModelMatrix[3]), mn->radius) > -1))) // World position is [3]
			{
				AnimatNode* an = dynamic_cast<AnimatNode*>(mn);

				if (an)  // Add delta animation time
					an->timeAccumulator += time.delta;

				pureDraw(current);
			}
		}
		else // If no model node render immediatley
			pureDraw(current);
	}
}

void RenderLoop::pureDraw(Node* current)
{
	current->draw();

	for (Node* child : current->children)
		draw(child);
}

void RenderLoop::checkGamePhaseKey(ModelNode* key)
{
		if (gamePhaseKey == false)
		{
			if (Frustum::getInstance()->inActionRadius(vec3(key->hirachicalModelMatrix[3])) == 1) 
			{			
				gamePhaseKey = true;
				Text::getInstance()->addText2Display(Text::KEY_FOUND);
				key->render = false;
				gameEventCheckIsOn = false;
			}
			else
			{
				Text::getInstance()->addText2Display(Text::KEY_NOTFOUND);
			}
		}
}

void RenderLoop::checkGamePhasePaper(ModelNode* paper)
{
	if (gamePhaseKey && gamePhasePaper == false)
	{
		if (Frustum::getInstance()->inActionRadius(vec3(paper->hirachicalModelMatrix[3])) == 1)
		{
			gamePhasePaper = true;
			gameEventCheckIsOn = false;
			Text::getInstance()->addText2Display(Text::PAPER_FOUND);
			//paper->render = false; //we dont want to let all papers disappear
		}
		else 
		{
			gameEventCheckIsOn = false; 
			Text::getInstance()->addText2Display(Text::PAPER_NOTFOUND);
		}
	}
}

void RenderLoop::checkGamePhaseEnd(ModelNode* zone)
{
	if (gamePhaseKey && gamePhasePaper && gameEventCheckIsOn && gamePhaseEnd == false)
	{
		if (Frustum::getInstance()->inActionRadius(vec3(zone->hirachicalModelMatrix[3])) == 1)
		{
			gamePhaseEnd = true;
			gameEventCheckIsOn = false;
			Text::getInstance()->addText2Display(Text::YOU_WON);
			//paper->render = false; //we dont want to let all papers disappear
		}
		else
		{
			gameEventCheckIsOn = false; //just do this once!
			Text::getInstance()->addText2Display(Text::PAPER_FOUND);
		}
	}
}

void RenderLoop::checkGameOverCondition(ModelNode* mn)
{
	Bullet* b = Bullet::getInstance();
	//+3.90f for setting the "camera of the enemy" to the height of the eyes
	vec3 enemyPosition = vec3(mn->hirachicalModelMatrix[3].x, mn->hirachicalModelMatrix[3].y + 3.90f, mn->hirachicalModelMatrix[3].z);
	
	/* No Radius detection for  gameplay
	// check always for the radius, e.g. player is too near
	vec3 targetDirectionPlayer = vec3(
		camera->position.x - enemyPosition.x,
		camera->position.y - enemyPosition.y,
		camera->position.z - enemyPosition.z); //we need the vector from Enemy -> Player
	vec3 targetNormalized = glm::normalize(targetDirectionPlayer); //and normalize it
	vec3 targetFinalPoint = enemyPosition + targetNormalized * 7.0f; //working radius for exmatriculation! Same procedure as with doors
																	
	 //jep, this has to be done that way with the raytest!
	btCollisionWorld::ClosestRayResultCallback res(btVector3(enemyPosition.x, enemyPosition.y, enemyPosition.z), btVector3(targetFinalPoint.x, targetFinalPoint.y, targetFinalPoint.z));
	b->getDynamicsWorld()->rayTest(btVector3(enemyPosition.x, enemyPosition.y, enemyPosition.z), btVector3(targetFinalPoint.x, targetFinalPoint.y, targetFinalPoint.z), res);
	if (res.hasHit() && res.m_collisionObject->CO_RIGID_BODY)
	{
		btRigidBody* body;
		body = (btRigidBody*)res.m_collisionObject;
		//see if it really the player that got hit and nothing else like a door
		if (body->getWorldArrayIndex() == camera->rigitBody->getWorldArrayIndex()) { //ArrayWorldIndex is the only thing that works and we have access to via methods
			printf("Player raycast hit in radius at: <%.2f, %.2f, %.2f>\n", res.m_hitPointWorld.getX(), res.m_hitPointWorld.getY(), res.m_hitPointWorld.getZ());
			gameOver = true; //only play sound once!
			Text::getInstance()->addText2Display(Text::GAME_OVER);
			SoundManager::getInstance()->playSound("Dialog\\exmatriculated.mp3");
		}
	} //check radius finished
	*/
	//check LineOfSight 
	vec3 activeSightPoint;
	if (!endOf1Reached)
		activeSightPoint = sightPoint1;
	else if (!endOf2Reached)
		activeSightPoint = sightPoint2;
	else if (!endOf3Reached)
		activeSightPoint = sightPoint3;
	else if (!endOf4Reached)
		activeSightPoint = sightPoint4;
  //TODO: Make Test of a range of Positions
	vec3 targetDirectionSightPoint = vec3(
		activeSightPoint.x - enemyPosition.x,
		activeSightPoint.y - enemyPosition.y,
		activeSightPoint.z - enemyPosition.z); //we need the vector from Enemy -> Sightpoint
	vec3 targetSightFinalPoint = enemyPosition + glm::normalize(targetDirectionSightPoint) * 16.0f; //working lineOfSight for exmatriculation! Same procedure as with doors

	btCollisionWorld::ClosestRayResultCallback resultSightLine(btVector3(enemyPosition.x, enemyPosition.y, enemyPosition.z), btVector3(targetSightFinalPoint.x, targetSightFinalPoint.y, targetSightFinalPoint.z));
	b->getDynamicsWorld()->rayTest(btVector3(enemyPosition.x, enemyPosition.y, enemyPosition.z), btVector3(targetSightFinalPoint.x, targetSightFinalPoint.y, targetSightFinalPoint.z), resultSightLine);
	if (resultSightLine.hasHit() && resultSightLine.m_collisionObject->CO_RIGID_BODY)
	{
		btRigidBody* body;
		body = (btRigidBody*)resultSightLine.m_collisionObject;
		//see if it really the player that got hit and nothing else like a door
		if (body->getWorldArrayIndex() == camera->rigitBody->getWorldArrayIndex()) { //ArrayWorldIndex is the only thing that works and we have access to via methods
			printf("Player raycast hit in sightline at: <%.2f, %.2f, %.2f>\n", resultSightLine.m_hitPointWorld.getX(), resultSightLine.m_hitPointWorld.getY(), resultSightLine.m_hitPointWorld.getZ());
			gameOver = true; //only play sound once!
			Text::getInstance()->addText2Display(Text::GAME_OVER);
			SoundManager::getInstance()->playSound("Dialog\\exmatriculated.mp3");
		}
	} //check sightOfLine finished

	
}


void RenderLoop::renderText()
{ // It is important to leave the if else structure here as it is
	if (fps)
		Text::getInstance()->fps(time.now, time.delta, drawnTriangles);

	if (wireFrameMode)
		Text::getInstance()->wireframe();

	if (showCamCoords)
		Text::getInstance()->showCamCoords(camera);

	if (drawBulletDebug)
		Text::getInstance()->drawBulletDebug();

	if (help)
		Text::getInstance()->help();
	else if (!render)
		Text::getInstance()->pause();
	else if (Text::getInstance()->hasTimeLeft())
		Text::getInstance()->removeTime(time.delta);

}

// Calculates the delta time, e.g. the time between frames, http://gafferongames.com/game-physics/fix-your-timestep/
void RenderLoop::calculateDeltaTime()
{
	time.now = glfwGetTime();
	time.lastDelta = time.delta;
	time.delta = time.now - time.past;

	if (time.delta > 0.25)
		time.delta = 0.25;

	time.past = time.now;
	time.accumulator += time.delta;

	Bullet* b = Bullet::getInstance();

	while (time.accumulator >= time.DIFFERNTIAL_DELTA)
	{
		b->getDynamicsWorld()->stepSimulation(time.DIFFERNTIAL_DELTA, time.BULLET_MAX_SUB_STEPS, time.BULLET_FIXED_TIME_STEP); // http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_the_World
		time.accumulator -= time.DIFFERNTIAL_DELTA;
	}

	const double alpha = time.accumulator / time.DIFFERNTIAL_DELTA;
	time.delta = time.delta * alpha + time.lastDelta * (1 - alpha); // weak https://en.wikipedia.org/wiki/Interpolation#Linear_interpolation

	Animator* a = ModelLoader::getInstance()->animator;
	a->UpdateAnimation(time.delta, a->ANIMATION_TICKS_PER_SECOND);
}



// Displays the ETU loading screen with music, source https://open.gl/textures
void RenderLoop::displayLoadingScreen(ModelLoader* ml) {
	unsigned int vao;// Create Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float vertices[] = {
		//Position(xy)Color(rgb)        Texcoords(xy)
		-0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
		0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
		0.8f, -0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
		-0.8f, -0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
	};

	unsigned int vbo;// Create a Vertex Buffer Object and copy the vertex data to it
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int ebo;// Create an element array
	glGenBuffers(1, &ebo);

	unsigned int elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	Shader* shader = new Shader("image");
	const unsigned int positionLocation = 0;      // Used in image.vert
	const unsigned int colorLocation = 1;         // Used in image.vert
	const unsigned int texCoordsLocation = 2;     // Used in image.vert
	const unsigned int uniformTextureLocation = 0;// Used in image.frag

	glEnableVertexAttribArray(positionLocation); // Specify the layout of the vertex data
	glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(texCoordsLocation);
	glVertexAttribPointer(texCoordsLocation, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));

	const string loadingImagePath = ml->MODEL_DIR + "Loading_Text.jpg"; // Load and bind textures
	const unsigned int loadingImageHande = ml->loadPicture(loadingImagePath);

	if (loadingImageHande < 0)
		Debugger::getInstance()->pauseExit("Malfunction: Loading image " + loadingImagePath + " not found.");

	shader->useProgram();
	const unsigned int textureIndex = 0;
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glUniform1i(uniformTextureLocation, textureIndex);//Paramas are location and texture unit
	glBindTexture(GL_TEXTURE_2D, loadingImageHande);

	// Loop activities
	glfwPollEvents(); // Check and call events
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear color buffer

	glfwGetWindowSize(window, &width, &height);
	// Draw a rectangle from the 2 triangles using 6 indices
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//Write text
	Text* t = Text::getInstance();
	t->init();
	t->loadingScreenInfo();
	//Text writing end
	glfwSwapBuffers(window);
	// Loop end

	// Del shader done in its deconstructor
	glDeleteTextures(1, &loadingImageHande);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void RenderLoop::toggleFullscreen()
{
	fullscreen = !fullscreen;

	if (fullscreen)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor(); // TODO buggy
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else
		glfwSetWindowMonitor(window, NULL, 0, 0, width, height, 0);
}

void RenderLoop::changeQuality()
{
	Text* text = Text::getInstance();
	int paramMin = 0;
	int paramMax = 0;

	if (textureSampling > 1) // Texture Sampling
		textureSampling = 0;

	if (textureSampling == 0)
	{
		text->addText2Display(Text::TEXTURE_SAMPLING_NEAREST_NEIGHBOR);
		paramMax = GL_NEAREST;
	}
	else //if (textureSampling == 1)
	{
		text->addText2Display(Text::TEXTURE_SAMPLING_BILINEAR);
		paramMax = GL_LINEAR;
	}

	if (mipMapping > 2) // Mip mapps
		mipMapping = 0;

	if (mipMapping == 0)
	{
		text->addText2Display(Text::MIP_MAPPING_OFF);
		paramMin = GL_NEAREST;
	}
	else if (mipMapping == 1)
	{
		text->addText2Display(Text::MIP_MAPPING_NEAREST_NEIGHBOR);

		if (textureSampling == 0)
			paramMin = GL_NEAREST_MIPMAP_NEAREST;
		else // textSampling == 1
			paramMin = GL_LINEAR_MIPMAP_NEAREST;
	}
	else if (mipMapping == 2)
	{
		text->addText2Display(Text::MIP_MAPPING_BILINEAR);

		if (textureSampling == 0)
			paramMin = GL_NEAREST_MIPMAP_LINEAR;
		else // textSampling == 1
			paramMin = GL_LINEAR_MIPMAP_LINEAR;
	}

	ModelLoader* ml = ModelLoader::getInstance();
	ml->setTextureState(ml->root, paramMin, paramMax);
}

double RenderLoop::getTimeDelta()
{
	return time.delta;
}