#pragma once
// Std. Includes
#include <vector>
// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		vec3 Position = vec3(0.0f, 0.0f, 0.0f); // Camera position
		vec3 Front; // vector pointing towards the camera's positive z-axis.
		vec3 up; // y axis, cross product of front and right
		vec3 Right; // represents the positive x-axis of the camera space
		vec3 WorldUp;
		// Eular Angles
		double Yaw = 0.0;
		double Pitch = 0.0;
		// Camera options
		double movementSpeed;
		double mouseSensitivity;
		double zoom;

		Camera(vec3 position, double _zoom, double _movementSpeed, double _mouseSensitivity);
		~Camera();

		mat4 getViewMatrix();
		void processKeyboard(Camera_Movement direction, double deltaTime);
		void processMouseScroll(double yoffset);
		void processMouseMovement(double xoffset, double yoffset);
};
