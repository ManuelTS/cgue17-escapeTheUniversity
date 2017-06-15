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
	if (action == GLFW_PRESS)
	{
		RenderLoop* rl = RenderLoop::getInstance();

		if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_END)) // See Text.cpp#help for keybindings
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_F1)
		{
			rl->render = !rl->render;
			rl->help = !rl->help;
		}
		else if (key == GLFW_KEY_F2)
			rl->fps = !rl->fps;
		else if (key == GLFW_KEY_F3)
			rl->wireFrameMode = !rl->wireFrameMode;
		else if (key == GLFW_KEY_F4)
		{
			rl->textureSampling++;
			rl->changeQuality();
		}
		else if (key == GLFW_KEY_F5)
		{
			rl->mipMapping++;
			rl->changeQuality();
		}
		else if (key == GLFW_KEY_F6)
			cout << "TODO: Visualizing the depth buffer." << endl; // TODO Visualizing the depth buffer http://learnopengl.com/#!Advanced-OpenGL/Depth-testing, swith shaders to depth ones
		else if (key == GLFW_KEY_F7 || key == GLFW_KEY_PAUSE)
			rl->render = !rl->render;
		else if (key == GLFW_KEY_F8)
		{
			rl->frustum = !rl->frustum;
			cout << "Switching Frustum Culling to ";
			if (rl->frustum)
				cout << "disabled." << endl;
			else
				cout << "enabled." << endl;
		}
		else if (key == GLFW_KEY_F9)
			rl->blending = !rl->blending;
		else if (key == GLFW_KEY_F10)
			rl->stencil = !rl->stencil;
		else if (key == GLFW_KEY_F11)
			rl->toggleFullscreen();
		//else if (key == GLFW_KEY_F12) // Causes an error
		else if(key == GLFW_KEY_SCROLL_LOCK)
			rl->drawBulletDebug = !rl->drawBulletDebug;
		else if (key == GLFW_KEY_SLASH || key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_RIGHT_BRACKET || key == GLFW_KEY_KP_ADD)
		{ // slash is german minus and right bracket is german plus on a german keyboard
			bool minus = key == GLFW_KEY_SLASH || key == GLFW_KEY_KP_SUBTRACT; // In- or decrease ambient light coefficient

			for (LightNode* ln : ModelLoader::getInstance()->lights)
				ln->light.diffuse.a += minus ? -0.01f : 0.01f;
		}
		else if (key == GLFW_KEY_E)
			rl->move(ModelLoader::getInstance()->root);
		else if (key == GLFW_KEY_Q)
		{
		
		}
		else if (key == GLFW_KEY_O) {
			Text::getInstance()->addText2Display(Text::GAME_OVER);
			SoundManager::getInstance()->playSound("Dialog\\exmatriculated.mp3");
		}
		else if (key == GLFW_KEY_B)
			SoundManager::getInstance()->playSound("Dialog\\burp.mp3");
		else if (key == GLFW_KEY_PRINT_SCREEN)
			Text::getInstance()->addText2Display(Text::SCREENY);
		else if (key == GLFW_KEY_BACKSLASH)
			rl->showCamCoords = !rl->showCamCoords;
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
	sm->initFileName("Music\\Jahzzar_-_01_-_The_last_ones.mp3"); // Init SM with music file to play while loading
	sm->playSound();

	initVar = new Initialization();

	width = initVar->width;
	height = initVar->height;
	glViewport(0, 0, width, height);

	camera = new Camera(glm::vec3(8,5,3), initVar->zoom, initVar->movingSpeed, initVar->mouseSensitivity);

	initGLFWandGLEW();
	displayLoadingScreen(ml);

	ml->load("Playground.dae"); // Load Models

	Bullet* b = Bullet::getInstance(); // Calculate bouding volumes, no pointer deletion since it is a singelton!
	b->init();
	b->createAndAddBoundingObjects(ml->root); // Sets pointers of rigitBodies in all nodes of the scene graph
	b->join();
	//b->createCamera(camera); // Create cam bounding cylinder

	//glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

	GBuffer* gBuffer = new GBuffer(initVar->maxWidth, initVar->maxHeight);
	ShadowMapping* realmOfShadows = new ShadowMapping();

	sm->stopAll(); // Stop loading sound
	while (!glfwWindowShouldClose(window)) // Start rendering
	{
		calculateDeltaTime();
		glfwPollEvents(); // Check and call events

		if (render)
		{
			doMovement(time.delta);
			doDeferredShading(gBuffer, realmOfShadows, ml);
		}
		else
		{
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		renderText();
		glfwGetWindowSize(window, &width, &height);
		glfwSwapBuffers(window);
	}

	sm->playSound("Dialog\\Bye.mp3"); // Exit sound
	Sleep(1600);

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

	Frustum* frustum = Frustum::getInstance();
	vector<LightNode::Light> renderingLights; // Contains the rendered lights
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE); // Must be before glClearColor, otherwise it remains untouched

	for (unsigned int i = 0; i < ml->lights.size(); i++) // Look which lights intersect or are in the frustum
	{
		LightNode* ln = ml->lights.at(i);
		const float sphereRadius = gBuffer->calcPointLightBSphere(ln); // Calculate the light sphere radius

		ln->light.position.w = frustum->sphereInFrustum(vec3(ln->light.position), sphereRadius);// See lightNode.hpp, use the radius of the light volume to cull lights not inside the frustum

		if (ln->light.position.w > -1) // Light volume intersects or is in frustum, render shadow map for light
			realmOfShadows->renderInDepthMap(ml->root, ln, sphereRadius, initVar->zoom, width, height); // far plane is the spheres radius

		renderingLights.push_back(ln->light);
	}

	// Deferred Shading
	Shader* gBufferShader = gBuffer->gBufferShader;

	// Deferred Shading: Geometry Pass, put scene's gemoetry/color data into gbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->handle); // Must be first!
	glViewport(0, 0, width, height);
	glDepthFunc(GL_LEQUAL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to white
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const float* projectionMatrixP = glm::value_ptr(glm::perspective(frustum->degreesToRadians(camera->zoom), (float)width / (float)height, frustum->nearD, frustum->farD));
	const float* viewMatrixP = glm::value_ptr(camera->getViewMatrix());
	gBufferShader->useProgram();
	glUniformMatrix4fv(gBufferShader->projectionLocation, 1, GL_FALSE, projectionMatrixP);
	glUniformMatrix4fv(gBufferShader->viewLocation, 1, GL_FALSE, viewMatrixP);
	draw(ml->root); // Draw all nodes except light ones

	if (drawBulletDebug)
		Bullet::getInstance()->debugDraw();

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);	

	// Light pass, point lights:
	Shader* deferredShader = gBuffer->deferredShader;
	deferredShader->useProgram();
	gBuffer->bindTextures();
	// Write light data to deferred shader
	glUniform3fv(deferredShader->viewPositionLocation, 1, &camera->position[0]);
	//Write shadow data to deferredShader.frag. 
	realmOfShadows->bindTexture(); // Link depth map into deferred Shader fragment
	glUniformMatrix4fv(realmOfShadows->SHADOW_LIGHT_SPACE_MATRIX_LOCATION, 1, GL_FALSE, glm::value_ptr(realmOfShadows->lightSpaceMatrix)); // Write the light space matrix to the deferred shader

	glBindBufferBase(GL_UNIFORM_BUFFER, ml->lightBinding, ml->lightUBO); // OGLSB: S. 169, always execute after new program is used
	glBindBuffer(GL_UNIFORM_BUFFER, ml->lightUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, renderingLights.size() * sizeof(renderingLights[0]), &renderingLights[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Now would come the directional light pass
	
	if (wireFrameMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gBuffer->renderQuad(); // Render 2D quad to screen
	realmOfShadows->unbindTexture();
	renderingLights.clear();
}

void RenderLoop::draw(Node* current)
{
	if (current && !dynamic_cast<LightNode*>(current)) // Don't draw light nodes
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);

		if (mn) // Leave if structure this way!
		{
			// If model node and (no frustum or transformationNode or modelNode bounding frustum sphere (modelNode center, radius) inside frustum):
			// ... render only when the model node schould be rendered

			if (mn->render && (frustum || Frustum::getInstance()->sphereInFrustum(mn->hirachicalModelMatrix[3], mn->radius) > -1)) // World position is [3]
			{
				AnimatNode* an = dynamic_cast<AnimatNode*>(mn);

				if (an) 
					an->timeAccumulator += time.delta;

				pureDraw(current);
			}
		}
		else // If no model node render anyway
			pureDraw(current);
	}
}

void RenderLoop::pureDraw(Node* current)
{
	current->draw();

	for (Node* child : current->children)
		draw(child);
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

// Calculates the delta time, e.g. the time between frames
void RenderLoop::calculateDeltaTime()
{
	time.now = glfwGetTime();
	time.delta = time.now - time.past;
	time.past = time.now;

	// Sets the timing syncronization of bullet physics, deltaTime is around 0.18
	Bullet::getInstance()->getDynamicsWorld()->stepSimulation(time.delta, 2, 0.16f); // Params: deltaTime, maxSubStepSize, fixedTimeStep in seconds. dt < msss * fts must hold!
}

/*Listens for user input.*/
void RenderLoop::doMovement(double timeDelta)
{
	//Set camera postion after the physics from the last frame were calculated
	btTransform trans;
	//camera->rigitBody->getMotionState()->getWorldTransform(trans);
	mat4 matrix;
	//trans.getOpenGLMatrix(glm::value_ptr(matrix));
	//camera->position = matrix[3];

	// Camera controls
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->processKeyboard(camera->FORWARD, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->processKeyboard(camera->BACKWARD, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->processKeyboard(camera->LEFT, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->processKeyboard(camera->RIGHT, timeDelta);

	//matrix[3] = vec4(camera->position, 1.0f); // Set the new position to the bounding camera object
	//trans.setFromOpenGLMatrix(glm::value_ptr(matrix));
	//camera->rigitBody->getMotionState()->setWorldTransform(trans);
}

// Displays the ETU loading screen with music, source https://open.gl/textures
void RenderLoop::displayLoadingScreen(ModelLoader* ml) {
	GLuint vao;// Create Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float vertices[] = {
		//Position(xy)Color(rgb)        Texcoords(xy)
		-0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
		0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // Top-right
		0.8f, -0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
		-0.8f, -0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
	};

	GLuint vbo;// Create a Vertex Buffer Object and copy the vertex data to it
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint ebo;// Create an element array
	glGenBuffers(1, &ebo);

	GLuint elements[] = {
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