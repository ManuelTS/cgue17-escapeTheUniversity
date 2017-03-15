#include "Frustum.hpp"


Frustum::Frustum()
{
}


Frustum::~Frustum()
{
}

int Frustum::pointInFrustum(glm::vec3 &p) {

	float pcz, pcx, pcy, aux;

	// compute vector from camera position to p
	glm::vec3 v = p - camPos;

	// compute and test the Z coordinate
	pcz = glm::dot(v,-Z);
	if (pcz > farD || pcz < nearD)

		return(OUTSIDE);

	// compute and test the Y coordinate
	pcy = glm::dot(v,Y);
	aux = pcz * tang;
	if (pcy > aux || pcy < -aux)
		return(OUTSIDE);

	// compute and test the X coordinate
	pcx = glm::dot(v,X);
	aux = aux * ratio;
	if (pcx > aux || pcx < -aux)
		return(OUTSIDE);

	return(INSIDE);
}

/*This function takes exactly the same parameters as the function gluPerspective.Each time the perspective definitions change, for instance when 
a window is resized, this function should be called as well.*/
void Frustum::setCamInternals(float angle, float ratio, float nearD, float farD) {

	// half of the the horizontal field of view
	float angleX;
	// store the information
	this->ratio = ratio;
	this->nearD = nearD;
	this->farD = farD;

	angle *= HALF_ANG2RAD;
	// compute width and height of the near and far plane sections
	tang = tan(angle);
	sphereFactorY = 1.0 / cos(angle);

	// compute half of the the horizontal field of view and sphereFactorX
	float anglex = atan(tang*ratio);
	sphereFactorX = 1.0 / cos(anglex);
}

/*This function takes three vectors that contain the information for the gluLookAt function: the position of the camera, 
a point to where the camera is pointing and the up vector. Each time the camera position or orientation changes, this function should be called as well. 
Notice how the following function is much simpler than for the other methods of view frustum culling. There is no need to compute the planes anymore.*/
void Frustum::setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u) {
	camPos = p;

	// compute the Z axis of the camera referential
	// this axis points in the same direction from
	// the looking direction
	Z = l - p;
	Z=glm::normalize(Z);

	// X axis of camera is the cross product of Z axis and given "up" vector 
	X = Z * u;
	X = glm::normalize(X);

	// the real "up" vector is the cross product of X and Z
	Y = X * Z;
}

int Frustum::sphereInFrustum(glm::vec3 &p, float radius) {

	float d;
	float az, ax, ay;
	int result = INSIDE;

	glm::vec3 v = p - camPos;

	az = glm::dot(v,-Z);
	if (az > farD + radius || az < nearD - radius)
		return(OUTSIDE);

	if (az > farD - radius || az < nearD + radius)
		result = INTERSECT;

	ay = glm::dot(v,Y);
	d = sphereFactorY * radius;
	az *= tang;
	if (ay > az + d || ay < -az - d)
		return(OUTSIDE);

	if (ay > az - d || ay < -az + d)
		result = INTERSECT;

	ax = glm::dot(v,X);
	az *= ratio;
	d = sphereFactorX * radius;
	if (ax > az + d || ax < -az - d)
		return(OUTSIDE);

	if (ax > az - d || ax < -az + d)
		result = INTERSECT;

	return(result);
}