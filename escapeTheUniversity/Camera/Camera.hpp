#pragma once
// Std. Includes
#include <vector>
// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "bullet/btBulletDynamicsCommon.h"

using namespace glm;

class Camera{
	private:
		void updateCameraVectors();

	public:
		// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
		enum Camera_Movement {
			FORWARD,
			BACKWARD,
			LEFT,
			RIGHT
		};

		// Camera Attributes
		vec3 position = vec3(0.0f, 0.0f, 0.0f); // Camera position
		vec3 front; // vector pointing towards the camera's positive z-axis.
		vec3 up; // y axis, cross product of front and right
		vec3 right; // represents the positive x-axis of the camera space
		vec3 worldUp;
		// Eular Angles
		double yaw = -90.0; //The yaw and pitch values are obtained from mouse (or controller/joystick) movement where horizontal mouse-movement affects the yaw and vertical mouse-movement affects the pitch.
		double pitch = 0.0;
		double lastX = 400; // Last x position of the mouse
		double lastY = 300; // Last y position of the mouse
		bool firstMouse = true; // True if the mouse is used for the first time, false if not
		// Camera options
		double movementSpeed;
		double mouseSensitivity;
		double zoom;
		// Bullet, first person camera rigit body
		btRigidBody* rigitBody;


		Camera(vec3 position, double _zoom, double _movementSpeed, double _mouseSensitivity);
		~Camera();

		mat4 getViewMatrix();
		void processKeyboard(Camera_Movement direction, double deltaTime);
		void processMouseScroll(double yoffset);
		void processMouseMovement(double x, double y);
};
