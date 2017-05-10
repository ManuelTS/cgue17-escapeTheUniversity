#include "SoundManager.hpp"
#include "Model/ModelLoader.hpp"
#include "Camera/Frustum.hpp"
#include "Model/Node/Node.hpp"
#include "Model/Node/ModelNode.hpp"
#include "Model/Node/LightNode.hpp"
#include "Model/Node/TransformationNode.hpp"
#include "RenderLoop.hpp"
#include "Camera/Camera.hpp"
#include "GBuffer.hpp"
#include "Shader.hpp"
#include "Text.hpp"
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
			cout << "TODO: Texture-Sampling-Quality: Nearest Neighbor/Bilinear" << endl; // TODO
		else if (key == GLFW_KEY_F5)
			cout << "TODO: Mip Maping-Quality: Off/Nearest Neighbour/Linear" << endl; // TODO
		else if (key == GLFW_KEY_F6)
			cout << "TODO: Visualizing the depth buffer." << endl; // TODO Visualizing the depth buffer http://learnopengl.com/#!Advanced-OpenGL/Depth-testing, swith shaders to depth ones
		else if (key == GLFW_KEY_F7 || key == GLFW_KEY_PAUSE)
			rl->render = !rl->render;
		else if (key == GLFW_KEY_F8) {
			rl->frustum = !rl->frustum;
			cout << "Switching Frustum Culling to ";
			if (rl->frustum == 1)
				cout << "disabled." << endl;
			else
				cout << "enabled." << endl;

		}
		else if (key == GLFW_KEY_F9)
			cout << "TODO: Blending on/off" << endl; // TODO
		else if (key == GLFW_KEY_F10)
			cout << "TODO:" << endl; // TODO
		else if (key == GLFW_KEY_F11)
			rl->toggleFullscreen();
		else if (key == GLFW_KEY_E)
		{
			for (Node* n : ModelLoader::getInstance()->getAllNodes())
			{
				TransformationNode* dn = dynamic_cast<TransformationNode*>(n);

				if (dn != nullptr)
					dn->switchState();
			}
		}
		else if (key == GLFW_KEY_Q)
		{
			// TODO
		}
		else if (key == GLFW_KEY_O) {
			Text::getInstance()->setDisplayTime(4000);
			SoundManager::getInstance()->playSound("Dialog\\exmatriculated.wav");
		}
		else if (key == GLFW_KEY_PRINT_SCREEN)
		{
			cout << "Hope you don't do anything bad with that screeny, sweetie." << endl;
		}
	}
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

	//instance->camera->processMouseScroll(instance->yScroll); // TODO ?
}

void mouseCallback(GLFWwindow* window, double x, double y)
{
	RenderLoop::getInstance()->camera->processMouseMovement(x,y);
}

void errorCallback(int error, const char* description)
{
	Debugger::getInstance()->errorCallback(error, description);
}

// Initializes GLFW and GLEW
void RenderLoop::initGLFWandGLEW(){
	if (!glfwInit())
		Debugger::getInstance()->pauseExit("Could not init GLFW.");

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
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
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
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
	ModelLoader* ml = ModelLoader::getInstance();
	SoundManager* sm = SoundManager::getInstance();
	sm->initFileName("Music\\Jahzzar_-_01_-_The_last_ones.mp3"); // Init SM with music file to play
	sm->playSound();

	initVar = new Initialization();
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f), initVar->zoom, initVar->movingSpeed, initVar->mouseSensitivity);

	initGLFWandGLEW();

	width = initVar->width;
	height = initVar->height;
	glViewport(0, 0, width, height);

	displayLoadingScreen(ml);

	Shader* gBufferShader = new Shader("gBuffer");
	Shader* deferredShader = new Shader("deferredShading");

	ml->load("Playground.dae");
	vec4 pos = ml->lights[9]->light.position;
	//pos.y -= 12.0f;
	//pos.z -= 70.0f;
    camera->position = glm::vec3(pos); // Set position of camera to the first light

	//glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

	GBuffer* gBuffer = new GBuffer(initVar->maxWidth, initVar->maxHeight);

	sm->stopAll(); // Stop loading sound
	while (!glfwWindowShouldClose(window)) // Start rendering
	{
		calculateDeltaTime();
		glfwPollEvents(); // Check and call events

		if (render)
		{
			doMovement(deltaTime);
			doDeferredShading(gBuffer, gBufferShader, deferredShader, ml);
		}
		else
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderText();
		glfwGetWindowSize(window, &width, &height);
		glfwSwapBuffers(window);
	}

	sm->playSound("Dialog\\Bye.mp3"); // Exit sound
	Sleep(1600);

	// TODO: Write with initVar game statistics, play time etc...
	delete gBuffer;
	glfwDestroyWindow(window);
	glfwTerminate();
}

void RenderLoop::doDeferredShading(GBuffer* gBuffer, Shader* gBufferShader, Shader* deferredShader, ModelLoader* ml) 
{
	if (wireFrameMode)
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clean color to white
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (fps)
		drawnTriangles = 0;

	// Deferred Shading: Geometry Pass, put scene's gemoetry/color data into gbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->handle); // Must be first!
	glViewport(0, 0, width, height);
	glDepthMask(GL_TRUE); // Must be before glClear()!
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clean color to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projectionMatrix = glm::perspective((float)camera->zoom, (float)width / (float)height, 0.1f, 100.0f);
	gBufferShader->useProgram();
	glUniformMatrix4fv(gBufferShader->projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(gBufferShader->viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
	draw(ml->root); // Draw all nodes except light ones
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Deferred Shading: Light Pass
	deferredShader->useProgram();
	gBuffer->bindTextures();
	// TODO: Performance optimization, only set one specific light if its position or light properties have changed, otherwise set all only once!
	// Bind buffer and fill all light node data in there
	vector<LightNode::Light> lights;

	for (LightNode* ln : ml->lights)
		lights.push_back(ln->light);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, ml->lightBinding, ml->lightUBO); // OGLSB: S. 169, always execute after new program is used
	glBindBuffer(GL_UNIFORM_BUFFER, ml->lightUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * sizeof(lights[0]), &lights[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glUniform3fv(deferredShader->viewPositionLocation, 1, &camera->position[0]);

	if (wireFrameMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// Render 2D quad
	gBuffer->renderQuad();
}

void RenderLoop::draw(Node* current)
{
	if (dynamic_cast<LightNode*>(current) == nullptr) // No light node, draw
	{
		ModelNode* mn = dynamic_cast<ModelNode*>(current);

		//TODO AABBs frustum culling, the used point one is inefficient but works
		//if (frustum || dynamic_cast<TransformationNode*>(current) != nullptr || mn != nullptr && Frustum::getInstance()->pointInFrustum(mn->position) != -1)
		//{ // TODO frustum not working, too much triangles drawn
			current->draw();

			for (Node* child : current->children)
				draw(child);
		//}
	}
}

void RenderLoop::renderText() 
{ // It is important to leave the if else structure here as it is
	if(fps)
		Text::getInstance()->fps(timeNow, deltaTime, drawnTriangles);

	if (help)
		Text::getInstance()->help();
	else if(!render)
		Text::getInstance()->pause();
	else if (Text::getInstance()->hasTimeLeft()) // Watch out, if more text becomes time dependent make an enum with the single times in it to set and render
		Text::getInstance()->gameOver(deltaTime);
}

// Calculates the delta time, e.g. the time between frames
void RenderLoop::calculateDeltaTime()
{
	timeNow = glfwGetTime();
	deltaTime = timeNow - timePast;
	timePast = timeNow;
}

/*Listens for user input.*/
void RenderLoop::doMovement(double timeDelta)
{
	// Camera controls
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->processKeyboard(camera->FORWARD, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->processKeyboard(camera->BACKWARD, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->processKeyboard(camera->LEFT, timeDelta);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->processKeyboard(camera->RIGHT, timeDelta);
}

// Displays the ETU loading screen with music, source https://open.gl/textures
void RenderLoop::displayLoadingScreen(ModelLoader* ml){
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