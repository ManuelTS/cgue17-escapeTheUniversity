#include "Frustum.hpp"

Frustum::~Frustum()
{
}

/*This function takes exactly the same parameters as the function gluPerspective.Each time the perspective definitions change, for instance when 
a window is resized, this function should be called as well.*/
void Frustum::setCamInternals(float angle, int width, int height)
{
	this->ratio = ((float)width) * 1.0 / ((float)height); // Calculate the ratio
	// compute width and height of the near and far plane sections
	tang = tan(angle * HALF_ANG2RAD / 2);
	height = nearD * tang;
	width = height * ratio;

	sphereFactorY = 1.0 / cos(angle);
	// compute half of the the horizontal field of view and sphereFactorX
	float anglex = atan(tang*ratio);
	sphereFactorX = 1.0 / cos(anglex);
}

/*This function takes three vectors that contain the information about the current camera, see Camera.cpp for more information.*/
void Frustum::setCamDef(glm::vec3 camPos, glm::vec3 front, glm::vec3 right, glm::vec3 up) {
	this->camPos = camPos;
	this->front = front;
	this->right = right;
	this->right = up;
}

int Frustum::pointInFrustum(glm::vec3 p)
{
	float pcz, pcx, pcy, aux;
	// compute vector from camera position to p
	glm::vec3 v = p - camPos;
	// compute and test the front coordinate
	pcz = glm::dot(v, this->front);

	if (pcz > farD || pcz < nearD)
		return-1;

	// compute and test the up coordinate
	pcy = glm::dot(v, this->right);
	aux = pcz * tang;

	if (pcy > aux || pcy < -aux)
		return-1;

	// compute and test the right coordinate
	pcx = glm::dot(v, this->right);
	aux = aux * ratio;

	if (pcx > aux || pcx < -aux)
		return-1;

	return 1;
}

int Frustum::sphereInFrustum(glm::vec3 p, float radius) {

	float d, az, ax, ay;
	int result = 1;
	glm::vec3 v = p - camPos;

	az = glm::dot(v, this->front);

	if (az > farD + radius || az < nearD - radius)
		return-1;

	if (az > farD - radius || az < nearD + radius)
		result = 0;

	ay = glm::dot(v, this->up);
	d = sphereFactorY * radius;
	az *= tang;

	if (ay > az + d || ay < -az - d)
		return-1;

	if (ay > az - d || ay < -az + d)
		result = 0;

	ax = glm::dot(v, this->right);
	az *= ratio;
	d = sphereFactorX * radius;

	if (ax > az + d || ax < -az - d)
		return-1;

	if (ax > az - d || ax < -az + d)
		result = 0;

	return result;
}

// TODO Adapt this to radar frustum culling when the bounding boxes from collision detection are available:
// Source http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
// signature is the bounding bos class
int Frustum::boxInFrustum() {

	int result = 1;
	//for each plane do ...
	for (int i = 0; i < 6; i++) {

		// is the positive vertex outside?
		/*if (pl[i].distance(b.getVertexP(pl[i].normal)) < 0)
			return -1;
		// is the negative vertex outside?
		else if (pl[i].distance(b.getVertexN(pl[i].normal)) < 0)
			result = 0;*/
	}
	return(result);
}