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
#include "SoundManager.hpp"
#include "Model/ModelLoader.hpp"
#include "Model/Node/Node.hpp"
#include "Model/Node/ModelNode.hpp"
#include "Model/Node/LightNode.hpp"
#include "Model/Node/TransformationNode.hpp"
#include "RenderLoop.hpp"
#include "Camera.hpp"
#include "GBuffer.hpp"
#include "Shader.hpp"
#include "Text.hpp"
#include "Debug/Debugger.hpp"
#include "Debug/MemoryLeakTracker.h"

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

		if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_END))
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_F1)
			cout << "TODO: Help" << endl; // TODO help
		else if (key == GLFW_KEY_F2){
			// TODO Display it on the screen not in console with own shader
			cout << printf("%s %8f %s %4.2f %s", "Frametime in MS:", rl->deltaTime * 1000, "~", 1.0 / rl->deltaTime, "FPS") << endl;
		}
		else if (key == GLFW_KEY_F3)// Wireframe on/off
		{
			if (rl->wireFrameMode == true){
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				rl->wireFrameMode = false;
			}
			else if (rl->wireFrameMode == false){ //has to be an else-if otherwise it turns it off again
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				rl->wireFrameMode = true;
			}
		}
		else if (key == GLFW_KEY_F4)
			cout << "TODO: Texture-Sampling-Quality: Nearest Neighbor/Bilinear" << endl;
		else if (key == GLFW_KEY_F5)
			cout << "TODO: Mip Maping-Quality: Off/Nearest Neighbour/Linear" << endl;
		else if (key == GLFW_KEY_F6)
			cout << "TODO: Visualizing the depth buffer." << endl; // TODO Visualizing the depth buffer http://learnopengl.com/#!Advanced-OpenGL/Depth-testing, swith shaders to depth ones
		else if (key == GLFW_KEY_F7 || key == GLFW_KEY_PAUSE){
			if (rl->render){
				cout << "Game is paused! " << endl;
			}
			else{
				cout << "Game is resumed! " << endl;
			}
			rl->render = !rl->render;
		}
		else if (key == GLFW_KEY_F8)
			cout << "TODO: Viewfrustum-Culling on/off" << endl;
		else if (key == GLFW_KEY_F9)
			cout << "TODO: Transparency on/off" << endl;
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
	glViewport(0, 0, width, height);

	//if (width > height) // OR THIS METHOD, todo
	//	glViewport((width - height) / 2, 0, min(width, height), min(width, height));

	//else
	//	glViewport(0, (height - width) / 2, min(width, height), min(width, height));
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

	//instance->camera->processMouseScroll(instance->yScroll);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	RenderLoop* instance = RenderLoop::getInstance();

	if (instance->firstMouse)
	{
		instance->lastX = xpos;
		instance->lastY = ypos;
		instance->firstMouse = false;
	}

	double xoffset = xpos - instance->lastX;
	double yoffset = instance->lastY - ypos;

	instance->lastX = xpos;
	instance->lastY = ypos;

	instance->camera->processMouseMovement(xoffset, yoffset);
}

void errorCallback(int error, const char* description)
{
	Debugger::getInstance()->errorCallback(error, description);
}

// Initializes GLFW and GLEW
void RenderLoop::initGLFWandGLEW(){
	if (!glfwInit())
		Debugger::getInstance()->pauseExit("Could not init GLFW.");

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Using OpenGL version 4.3, 4.4 could be used if necessary
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, initVar->fps);

	window = glfwCreateWindow(initVar->width, initVar->height, initVar->windowTitle, initVar->fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

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
		cout << endl << "Working on:" << endl;
		cout << glGetString(GL_VENDOR) << endl;
		cout << glGetString(GL_VERSION) << endl;
		cout << glGetString(GL_RENDERER) << endl;
		cout << "OpenGL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl << endl;
		int param = 0;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &param);
		cout << "Maximal 3D Texture Size: " << param << endl;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &param);
		cout << "Maximal texture size: " << param << endl;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &param);
		cout << "Maximal texture image units: " << param << endl;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &param);
		cout << "Maximal uniform buffer binding points: " << param << endl;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &param);
		cout << "Maximal uniform block size: " << param << endl;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &param);
		cout << "Maximal color attachments: " << param << endl;
		glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &param);
		cout << "Maximal framebuffer height: " << param << endl;
		glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &param);
		cout << "Maximal framebuffer width: " << param << endl;
		glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &param);
		cout << "Maximal framebuffer samples: " << param << endl;
		glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &param);
		cout << "Maximal framebuffer layers: " << param << endl << endl;

		const unsigned int charArrayLength = 2;
		unsigned char chars[charArrayLength] = { Text::FIRST_CHARACTER, Text::LAST_CHARACTER };

		cout << "Supported extended ASCII characters from to are:"<< endl;

		for (unsigned int i = 0; i < charArrayLength; i++)
			printf("character: '%c' with ASCII decimal value: %2i\n", chars[i], (unsigned int)chars[i]);
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
}

void RenderLoop::start()
{ // Init all
	SoundManager* sm = SoundManager::getInstance();
	sm->initFileName("Sounds/Music/Jahzzar_-_01_-_The_last_ones.mp3"); // Init SM with music file to play
	sm->playSound();

	initVar = new Initialization();
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f), initVar->zoom, initVar->movingSpeed, initVar->mouseSensitivity);

	initGLFWandGLEW();

	width = initVar->width;
	height = initVar->height;

	glViewport(0, 0, width, height);

	ModelLoader* ml = ModelLoader::getInstance();
	displayLoadingScreen(ml);
	Shader* gBufferShader = new Shader(".\\Shader\\gBuffer.vert", ".\\Shader\\gBuffer.frag");
	Shader* deferredShader = new Shader(".\\Shader\\deferredShading.vert", ".\\Shader\\deferredShading.frag");
	ml->load(".\\Models\\Playground.dae");

	//glEnable(GL_CULL_FACE);
	//glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

	GBuffer* gBuffer = new GBuffer(initVar->maxWidth, initVar->maxHeight);
	
	Text* text = Text::getInstance();
	text->init();

	sm->stopAll(); // Stop loading sound
	while (!glfwWindowShouldClose(window)) // Start rendering
	{
		calculateDeltaTime();
		glfwPollEvents(); // Check and call events

		if (render){
			doMovement(deltaTime);

			// Deferred Shading: Geometry Pass, put scene's gemoetry/color data into gbuffer
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->handle); // Must be first!
			glViewport(0, 0, width, height);
			glDepthMask(GL_TRUE); // Must be before glClear()!
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
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
			drawLights(ml);
			glUniform3fv(deferredShader->viewPositionLocation, 1, &camera->Position[0]);

			// Render 2D quad
			gBuffer->renderQuad();

			//text->write("Ha");
		}

		glfwGetWindowSize(window, &width, &height);
		glfwSwapBuffers(window);
	}

	sm->playSound("Sounds/Dialog/Bye.mp3"); // Exit sound
	Sleep(1600);

	// TODO: Write with initVar game statistics, play time etc...
	delete gBuffer;
	glfwDestroyWindow(window);
	glfwTerminate();
}

void RenderLoop::draw(Node* current)
{
	if (dynamic_cast<LightNode*>(current) == nullptr) // No light node, draw
	{
		current->draw();

		for (Node* child : current->children)
			draw(child);
	}
}

// Draws all lights
void RenderLoop::drawLights(ModelLoader* ml)
{
	// TODO: Performance optimization, only set one specific light if its position or light properties have changed, otherwise set all only once!
	// Bind buffer and fill all light node data in there
	vector<LightNode::Light> lights;

	for (LightNode* ln : ml->lights){
		lights.push_back(ln->light);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, ml->lightBinding, ml->lightUBO); // OGLSB: S. 169, always execute after new program is used
	glBindBuffer(GL_UNIFORM_BUFFER, ml->lightUBO);
 	glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * sizeof(lights[0]), &lights[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

	Shader* shader = new Shader(".\\Shader\\image.vert", ".\\Shader\\image.frag");
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

	const string loadingImagePath = "Models/Loading_Text.jpg"; // Load and bind textures
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
	glClearColor(0.0f, 0.0f, .0f, 1.0f); // Set clean color to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear color buffer

	glfwGetWindowSize(window, &width, &height);
	// Draw a rectangle from the 2 triangles using 6 indices
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glfwSwapBuffers(window);
	// Loop end

	// Del shader done in its deconstructor
	glDeleteTextures(1, &loadingImageHande);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
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