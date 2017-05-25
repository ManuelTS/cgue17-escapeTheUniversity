#pragma once
#include "Initialization.hpp"
#include <GLM\glm.hpp>
#include <vector>

class Initialization;
class Shader;
class Camera;
class Node;
class ModelNode;
class ModelLoader;
class GBuffer;

/*Manages all parts of the game and contains the render loop. This class has the singelton design pattern implemented.*/
class RenderLoop{
	private:
		GLFWwindow* window;
		// Time
		double timePast; // Last, past time  in milliseconds
		double timeNow; // New, current time in milliseconds
	
		RenderLoop(void){}; // Private constructor to allow only one instance
		RenderLoop(RenderLoop const&); // Private constructor to prevent copies
		void operator=(RenderLoop const&); // Private constructor to prevent assignments
		void initGLFWandGLEW(); // Initializes GLFW and GLEW
		void doMovement(double timeDelta);
		void doDeferredShading(GBuffer* gBuffer, Shader* gBufferShader, Shader* deferredShader, ModelLoader* ml); // Does the deferred shading gemetry and lighning pass and draws the screen quad afterwards
		void calculateDeltaTime(); // Calculates the delta time, e.g. the time between frames
		void draw(Node* current); // Draws all lights which checking their dependencies and condition (no Light Node drawing, frustum culling, flag setting)
		void pureDraw(Node* current); // Draws the argument node without any checks and calls draw for its children again
		void displayLoadingScreen(ModelLoader* ml); // Displays the loading screen
	public:
		Camera* camera;
		Initialization* initVar;

		int width; // Current window width
		int height; // Current window height
		double xScroll = 0.0; // The last x value scrolled
		double yScroll = 0.0; // The last y value scrolled
		unsigned int drawnTriangles = 0;// Contains all the drawn triangles if the fps mode is activated
		bool help = false; // Display help on screen or not, standard false
		bool fps = false; // Display FPS on screen or, standard false
		bool wireFrameMode = false; // True if the wireframe mode is activated, false if not, standard false
		bool render = true; // Stop or start rendering with a key, standard true
		bool frustum = false; // Toggle frustum culling, standard false
		bool blending = true; // Toggle usage of blending, default true
		bool stencil = true; // Toogle usage of the stencil buffer, default true
		bool fullscreen = false; // Toogle fullscreen mode, default false
		bool showCamCoords = false; // Toggle the rendering of the camera coords on the screen, default false
		
		double deltaTime; // Difference of variable timeNew and timeOld in nano seconds

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