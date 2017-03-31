#include "Camera.hpp"
#include "Model\Frustum.hpp"
#include "Debug\MemoryLeakTracker.h"

Camera::~Camera()
{
}

// Constructor with vectors
Camera::Camera(glm::vec3 position, double _zoom, double _movementSpeed, double _mouseSensitivity) : Position(position), zoom(_zoom), movementSpeed(_movementSpeed), mouseSensitivity(_mouseSensitivity), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)), Front(glm::vec3(0.0f, 0.0f, -1.0f))
{
	this->updateCameraVectors();
}

// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(this->Position, this->Position + this->Front, this->up);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(Camera_Movement direction, double deltaTime)
{
	GLfloat velocity = this->movementSpeed * deltaTime;

	if (direction == FORWARD)
		this->Position += this->Front * velocity;
	if (direction == BACKWARD)
		this->Position -= this->Front * velocity;
	if (direction == LEFT)
		this->Position -= glm::normalize(glm::cross(this->Front, this->up)) * velocity;
	if (direction == RIGHT)
		this->Position += glm::normalize(glm::cross(this->Front, this->up)) * velocity;

	Frustum::getInstance()->setCamDef(this->Position, this->Front, this->Right, this->up);
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(double xoffset, double yoffset)
{
	xoffset *= this->mouseSensitivity;
	yoffset *= this->mouseSensitivity;

	this->Yaw += xoffset;
	this->Pitch += yoffset;

	// update Front, Right and up Vectors using the updated Eular angles
	this->updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::processMouseScroll(double yoffset)
{
	const double MAX_ZOOM = 45.0;

	if (this->zoom >= 1.0f && this->zoom <= MAX_ZOOM)
		this->zoom -= yoffset;

	if (this->zoom <= 1.0f)
		this->zoom = 1.0f;

	if (this->zoom > MAX_ZOOM)
		this->zoom = MAX_ZOOM;

	Frustum::getInstance()->setCamDef(this->Position, this->Front, this->Right, this->up);
}

// Calculates the front vector from the Camera's (updated) Eular Angles
void Camera::updateCameraVectors()
{
	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (this->Pitch > 89.0f)
		this->Pitch = 89.0f;
	if (this->Pitch < -89.0f)
		this->Pitch = -89.0f;

	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	front.y = sin(glm::radians(this->Pitch));
	front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	this->up = glm::normalize(glm::cross(this->Right, this->Front));

	Frustum::getInstance()->setCamDef(this->Position, this->Front, this->Right, this->up);
}