#pragma once
#include <GLM\glm.hpp>

// Calculates if a point or shepre is in, out, or intersects the view frustum with the radar approach
class Frustum
{
private:
	const float HALF_ANG2RAD = 3.14159265358979323846f / 360.0f;
	glm::vec3 camPos; // camera position
	glm::vec3 X, Y, Z; // the camera referential
	float nearD; // Distance from the camera to the near plane
	float farD; // Distance from the camera to the far plane
	float width, height, ratio;
	double tang;
	float sphereFactorX, sphereFactorY;

public:
	enum { OUTSIDE, INTERSECT, INSIDE };

	Frustum();
	~Frustum();

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u);
	int pointInFrustum(glm::vec3 &p);
	int Frustum::sphereInFrustum(glm::vec3 &p, float radius);
};



