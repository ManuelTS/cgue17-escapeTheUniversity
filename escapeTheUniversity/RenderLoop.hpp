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
		
		struct Time // variables of this game, approach of http://gafferongames.com/game-physics/fix-your-timestep/
		{
			double past; // Last, past time  in mili or nano seconds, according to glfw its system dependend
			double now; // New, current time in mili or nano seconds, according to glfw its system dependend
			double delta; // Difference of variable timeNew and timeOld in mili or nano seconds, according to glfw its system dependend
			const double differentialDelta = 0.01; //dt, The renderer produces time and the simulation consumes it in discrete dt sized chunks.
			double accumulator = 0.0; // Notice that we only process with steps sized dt. Hence, in the common case we have some unsimulated time left over at the end of each frame passed on to the next frame via the accumulator variable and is not thrown away.

		} time;
	
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

		//Switches and modes for game
		bool help = false; // Display help on screen or not, standard false
		bool fps = false; // Display FPS on screen or, standard false
		bool wireFrameMode = false; // True if the wireframe mode is activated, false if not, standard false
		bool render = true; // Stop or start rendering with a key, standard true
		bool frustum = false; // Toggle frustum culling, standard false
		bool blending = true; // Toggle usage of blending, default true
		bool stencil = true; // Toogle usage of the stencil buffer, default true
		bool fullscreen = false; // Toogle fullscreen mode, default false
		bool showCamCoords = false; // Toggle the rendering of the camera coords on the screen, default false
		bool drawBulletDebug = false; // Toggle the drawing of the bullet world debug, default false
		// Sampling states
		int textureSampling = 1; // Texture Sampling Quality: 0 = Nearest Neighbor, 1 = Bilinear, default: 1 see https://www.informatik-forum.at/showthread.php?107156-Textur-Sampling-Mip-Mapping
		int mipMapping = 2; // Maping-Quality: 0 = Off, 1 = Nearest Neigbor, 2 = Bilinear, default: 2
		

		~RenderLoop();
		void start(); // Initializes and starts the actual renderloop
		void renderText(); // Renders the FPS on the screen
		void toggleFullscreen(); // Toggles the fullscreen
		void changeQuality(); // Changes the quality of texture sampling and Mip Mapping, see variables mipMapping and textureSampling. True if texture, false if mip mapping
		double getTimeDelta(); // Returns the current delta time

		/*Returns the pointer to the unique instance of the render loop class.*/
		static RenderLoop* RenderLoop::getInstance()
		{
			static RenderLoop instance;// lazy singleton, instantiated on first use
			return &instance;
		}
};