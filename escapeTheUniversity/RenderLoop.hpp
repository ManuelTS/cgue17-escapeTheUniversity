#pragma once
#include <GLM\glm.hpp>
#include <vector>
#include "Initialization.hpp"

class Initialization;
class Shader;
class Camera;
class Node;
class ModelLoader;
class GBuffer;

/*Manages all parts of the game and contains the render loop. This class has the singelton design pattern implemented.*/
class RenderLoop{
	private:
		GLFWwindow* window;
		Initialization* initVar;
		// Time
		double timePast; // Last, past time
		double timeNow; // New, current time

		RenderLoop(void){}; // Private constructor to allow only one instance
		RenderLoop(RenderLoop const&); // Private constructor to prevent copies
		void operator=(RenderLoop const&); // Private constructor to prevent assignments
		void initGLFWandGLEW(); // Initializes GLFW and GLEW
		void doMovement(double timeDelta);
		void doDeferredShading(GBuffer* gBuffer, Shader* gBufferShader, Shader* deferredShader, ModelLoader* ml); // Does the deferred shading gemetry and lighning pass and draws the screen quad afterwards
		void calculateDeltaTime(); // Calculates the delta time, e.g. the time between frames
		void draw(Node* current); // Draws all lights except light nodes
		void displayLoadingScreen(ModelLoader* ml);
	public:
		Camera* camera;

		int width; // Current window width
		int height; // Current window height
		double xScroll = 0.0; // The last x value scrolled
		double yScroll = 0.0; // The last y value scrolled
		double lastX = 400;
		double lastY = 300;
		bool firstMouse = true; // True if the mouse is used for the first time, false if not
		bool render = true; // Stop or start rendering with a key
		bool wireFrameMode = false; // True if the wireframe mode is activated, false if not
		bool fps = false; // Display FPS on screen or not
		bool help = false; // Display help on screen or not
		
		double deltaTime; // Difference of variable timeNew and timeOld

		~RenderLoop();
		void start(); // Initializes and starts the actual renderloop
		void renderText(); // Renders the FPS on the screen

		/*Returns the pointer to the unique instance of the render loop class.*/
		static RenderLoop* RenderLoop::getInstance()
		{
			static RenderLoop instance;// lazy singleton, instantiated on first use
			return &instance;
		}
};