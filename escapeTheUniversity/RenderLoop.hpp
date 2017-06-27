#pragma once
#define GLM_FORCE_RADIANS // Use this for radiant calculation, the GLM one does not work!

#include "Initialization.hpp"
#include "ShadowLight\GBuffer.hpp"
#include "ShadowLight\ShadowMapping.hpp"
#include <GLM\glm.hpp>
#include <vector>

class Camera;
class Node;
class ModelNode;
class ModelLoader;

/*Manages all parts of the game and contains the render loop. This class has the singelton design pattern implemented.*/
class RenderLoop
{
	private:
		GLFWwindow* window;
	
		RenderLoop(void){}; // Private constructor to allow only one instance
		RenderLoop(RenderLoop const&); // Private constructor to prevent copies
		void operator =(RenderLoop const&); // Private constructor to prevent assignments
		void initGLFWandGLEW(); // Initializes GLFW and GLEW
		void doMovement(double timeDelta);
		void doDeferredShading(GBuffer* gBuffer, ShadowMapping* realmOfShadows, ModelLoader* ml); // Does the deferred shading gemetry and lighning pass and draws the screen quad afterwards
		void calculateDeltaTime(); // Calculates the delta time, e.g. the time between frames
		void draw(Node* current); // Draws all lights which checking their dependencies and condition (no Light Node drawing, frustum culling, flag setting)
		void displayLoadingScreen(ModelLoader* ml); // Displays the loading screen
		void checkGameOverCondition(ModelNode* mn); //checks if the play is too near of the enemy or in sight

 		
	public:
		struct Time // variables of this game, approach of http://gafferongames.com/game-physics/fix-your-timestep/
		{
			double past; // Last, past time  in mili or nano seconds, according to glfw its system dependend
			double now; // New, current time in mili or nano seconds, according to glfw its system dependend
			double lastDelta; // Difference of variable timeNew and timeOld in mili or nano seconds, according to glfw its system dependend from the last run
			double delta; // Difference of variable timeNew and timeOld in mili or nano seconds, according to glfw its system dependend
			const double DIFFERNTIAL_DELTA = 0.01; //dt, The renderer produces time and the simulation consumes it in discrete dt sized chunks.
			double accumulator = 0.0; // Notice that we only process with steps sized dt. Hence, in the common case we have some unsimulated time left over at the end of each frame passed on to the next frame via the accumulator variable and is not thrown away.
			const double BULLET_MAX_SUB_STEPS = 1.0; //  is the maximum number of steps that Bullet is allowed to take each time you call it. If you pass a very large timeStep as the first parameter [say, five times the size of the fixed internal time step], then you must increase the number of maxSubSteps to compensate for this, otherwise your simulation is “losing” time.
			const double BULLET_FIXED_TIME_STEP = 1.0 / 60.0; // Bullet maintains an internal clock, in order to keep the actual length of ticks constant.
			double animationAccumulator = 0.0; // Accumulates the animation time
		} time;

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
		bool drawLightBoundingSpheres = false; // Toogle the rendering of the light sources bounding spheres, default false
		bool drawBulletDebug = false; // Toggle the drawing of the bullet world debug, default false
		bool drawShadowMap = false; // Toogle the drawing of a shadow map on screen, default false
		bool freeCamera = false; // Toogles the camera to its bullet collision shape, default false
		bool disableKeyRendering = false; //indicates the pickup of the key

		bool gameEventCheckIsOn = false; //for going only through the check 1 frame. is reset every loop
		bool gamePhaseKey = false; //indicates the pickup of the key
		void checkGamePhaseKey(ModelNode* key);  //indicates the game-section of the key
		bool gamePhasePaper = false; //indicates the pickup of the exam
		void checkGamePhasePaper(ModelNode* paper);  //indicates the game-section of the paper
		bool gamePhaseEnd = false; // reached the safe zone
		void checkGamePhaseEnd(ModelNode* paper);  //indicates the game-section of the end
		bool gameOver = false; //indicates if you have been caught and game is over!

		glm::vec3 sightPoint1 = glm::vec3(-5.7f, 2.3f, 14.0f); //View-Direction Node at center of d09_hinge for sight of enemy
		glm::vec3 sightPoint2 = glm::vec3(24.0f, 2.3f, 11.75f); //View-Direction Node at edge of OfficeDesk.003
		glm::vec3 sightPoint3 = glm::vec3(20.5f, 2.3f, -15.0f); //View-Direction Node near at Poster3.002
		glm::vec3 sightPoint4 = glm::vec3(-13.4f, 2.3f, -14.1f); //View-Direction Node-Point near to DoorFrameSteel

		glm::vec3 walkPoint1 = glm::vec3(-5.7f, -0.3275f, 12.0f);
		glm::vec3 walkPoint2 = glm::vec3(20.5f, -0.3275f, 12.0f);
		glm::vec3 walkPoint3 = glm::vec3(20.5f, -0.3275f, -13.75f);
		glm::vec3 walkPoint4 = glm::vec3(-5.7f, -0.3275f, -13.75f);
		
		bool endOf1Reached = false; //inidicates that we have to turn and go over to the next point (2)
		bool endOf2Reached = false; //inidicates that we have to turn and go over to the next point (3)
		bool endOf3Reached = false; //inidicates that we have to turn and go over to the next point (4)
		bool endOf4Reached = false; //inidicates that we have to turn and go over to the next point (1) 


		// Sampling states
		int textureSampling = 1; // Texture Sampling Quality: 0 = Nearest Neighbor, 1 = Bilinear, default: 1 see https://www.informatik-forum.at/showthread.php?107156-Textur-Sampling-Mip-Mapping
		int mipMapping = 2; // Maping-Quality: 0 = Off, 1 = Nearest Neigbor, 2 = Bilinear, default: 2
		

		~RenderLoop();
		void start(); // Initializes and starts the actual renderloop
		void pureDraw(Node* current); // Draws the argument node without any checks and calls draw for its children again
		void renderText(); // Renders the FPS on the screen
		void toggleFullscreen(); // Toggles the fullscreen
		void changeQuality(); // Changes the quality of texture sampling and Mip Mapping, see variables mipMapping and textureSampling. True if texture, false if mip mapping
		double getTimeDelta(); // Returns the current delta time

		void move(Node* current);//temp method to move all doors

		/*Returns the pointer to the unique instance of the render loop class.*/
		static RenderLoop* RenderLoop::getInstance()
		{
			static RenderLoop instance;// lazy singleton, instantiated on first use
			return &instance;
		}
};