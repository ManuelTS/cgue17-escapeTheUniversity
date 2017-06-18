#include "Camera.hpp"
#include "Frustum.hpp"
#include "..\Debug\MemoryLeakTracker.h"

Camera::~Camera()
{
}

// Constructor with vectors
Camera::Camera(glm::vec3 position, double _zoom, double _movementSpeed, double _mouseSensitivity) : position(position), zoom(_zoom), movementSpeed(_movementSpeed), mouseSensitivity(_mouseSensitivity), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), front(glm::vec3(0.0f, 0.0f, 0.0f))
{
	updateCameraVectors();
}

// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(Camera_Movement direction, double deltaTime)
{
	float velocity = movementSpeed * deltaTime;

	if (direction == FORWARD)
		position += front * velocity;
	if (direction == BACKWARD)
		position -= front * velocity;
	if (direction == LEFT)
		position -= glm::normalize(glm::cross(front, up)) * velocity;
	if (direction == RIGHT)
		position += glm::normalize(glm::cross(front, up)) * velocity;
	if (direction == UP)
		position.y += 1.5f;

	Frustum::getInstance()->setCamDef(position, front, right, up);
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(double x, double y)
{
	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	double xoffset = (x - lastX) * mouseSensitivity;
	double yoffset = (lastY - y) * mouseSensitivity;

	lastX = x;
	lastY = y;

	yaw += xoffset;
	pitch += yoffset;

	// update front, right and up Vectors using the updated Eular angles
	updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::processMouseScroll(double yoffset)
{
	const double MAX_ZOOM = 45.0;

	if (zoom >= 1.0f && zoom <= MAX_ZOOM)
		zoom -= yoffset;

	if (zoom <= 1.0f)
		zoom = 1.0f;

	if (zoom > MAX_ZOOM)
		zoom = MAX_ZOOM;
}

// Calculates the front vector from the Camera's (updated) Eular Angles
void Camera::updateCameraVectors()
{
	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Calculate the new front vector
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	front = glm::normalize(front);
	// Also re-calculate the right and Up vector
	right = glm::normalize(glm::cross(worldUp, front));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::cross(front, right); // Both already normalized

	Frustum::getInstance()->setCamDef(position, front, right, up);

}