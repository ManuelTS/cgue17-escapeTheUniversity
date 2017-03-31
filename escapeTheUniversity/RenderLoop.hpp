#pragma once
#include "Initialization.hpp"
#include <GLM\glm.hpp>
#include <vector>

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
		void displayLoadingScreen(ModelLoader* ml); // Displays the loading screen
	public:
		Camera* camera;

		int width; // Current window width
		int height; // Current window height
		double xScroll = 0.0; // The last x value scrolled
		double yScroll = 0.0; // The last y value scrolled
		double lastX = 400; // Last x position of the mouse
		double lastY = 300; // Last y position of the mouse
		unsigned int drawnTriangles = 0;// Contains all the drawn triangles if the fps mode is activated
		bool firstMouse = true; // True if the mouse is used for the first time, false if not
		bool help = false; // Display help on screen or not, standard false
		bool fps = false; // Display FPS on screen or, standard false
		bool wireFrameMode = false; // True if the wireframe mode is activated, false if not, standard false
		bool render = true; // Stop or start rendering with a key, standard true
		bool frustum = false; // Toggle frustum culling, standard false
		bool fullscreen = false; // Toogle fullscreen mode
		
		double deltaTime; // Difference of variable timeNew and timeOld

		~RenderLoop();
		void start(); // Initializes and starts the actual renderloop
		void renderText(); // Renders the FPS on the screen
		void toggleFullscreen(); // Toggles the fullscreen

		/*Returns the pointer to the unique instance of the render loop class.*/
		static RenderLoop* RenderLoop::getInstance()
		{
			static RenderLoop instance;// lazy singleton, instantiated on first use
			return &instance;
		}
};