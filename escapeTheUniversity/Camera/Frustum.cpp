#include "Frustum.hpp"

Frustum::~Frustum()
{
}

/*This function takes exactly the same parameters as the function gluPerspective.Each time the perspective definitions change, for instance when 
a window is resized, this function should be called as well.*/
void Frustum::setCamInternals(float angle, int width, int height)
{
	ratio = ((float)width) * 1.0 / ((float)height); // Calculate the ratio
	angle = degreesToRadians(angle) / 2;
	// compute width and height of the near and far plane sections
	tang = tan(angle);

	sphereFactorY = 1.0 / cos(angle); // Only for spheres, compute a part of the distance and multiply it later with the sphere radius
	// compute half of the the horizontal field of view and sphereFactorX
	const float anglex = atan(tang * ratio);
	sphereFactorX = 1.0 / cos(anglex);
}

/*This function takes three vectors that contain the information about the current camera, see Camera.cpp for more information.*/
void Frustum::setCamDef(glm::vec3 camPos, glm::vec3 front, glm::vec3 up) {
	this->camPos = camPos;
	this->front = glm::normalize(front - camPos);
	this->right = glm::normalize(glm::cross(this->front, up));
	this->up = glm::cross(this->right, this->front);
}

int Frustum::pointInFrustum(glm::vec3 p)
{
	glm::vec3 fromCam2P = p - camPos; // compute vector from camera position to p
	const float frontDistance = glm::dot(front, fromCam2P); // compute and test the front coordinate

	if (frontDistance < nearD || frontDistance > farD)
		return -1;

	const float upperLine = glm::dot(up, fromCam2P); // compute and test the up coordinate
	float border = frontDistance * tang; // Calculate the upper border

	if (upperLine < -border || upperLine > border)
		return -1;

	const float rightLine = glm::dot(right, fromCam2P);// compute and test the right coordinate
	border = border * ratio; // Calculate the side border

	if (rightLine < -border || rightLine > border)
		return -1;

	return 1;
}

int Frustum::sphereInFrustum(const glm::vec3 sphereCenter, const float radius)
{
	int result = 1; // Return value, 1 inside the frustum, 0 intersction, -1 outside the frustum
	const glm::vec3 fromCam2Center = sphereCenter - camPos; // Calculate distance
	const float az = glm::dot(front, fromCam2Center);

	if (az > farD + radius || az < nearD - radius)
		return -1;

	const float ax = glm::dot(fromCam2Center, right);
	const float zz1 = az * tang * ratio;
	const float distance1 = sphereFactorX * radius;
	
	if (ax > zz1 + distance1 || ax < -zz1 - distance1)
		return -1;

	const float ay = glm::dot(fromCam2Center, up);
	const float zz2 = az * tang;
	const float distance2 = sphereFactorY * radius; // Distance from sphere center to frustum plane, not radius since a sphere tangent to the frustum can have a greater distance
	
	if (ay > zz2 + distance2 || ay < -zz2 - distance2)
		return -1;

	if (az > farD - radius || az < nearD + radius)
		result = 0;
	if (ay > zz2 - distance2 || ay < -zz2 + distance2)
		result = 0;
	if (ax > zz1 - distance1 || ax < -zz1 + distance1)
		result = 0;

	return result;
}

float Frustum::degreesToRadians(float degrees)
{
	return degrees * (3.141592f / 180.0f); // Use this for radiant calculation, the GLM one does not work!
}