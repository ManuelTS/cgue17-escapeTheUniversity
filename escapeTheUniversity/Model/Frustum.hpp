#pragma once
#include <GLM\glm.hpp>

// Calculates if a point or shepre is in, out, or intersects the view frustum with the radar approach
class Frustum
{
private:
	const float nearD = 1.0f; // Distance from the camera to the near plane
	const float farD = 100.0f; // Distance from the camera to the far plane
	const float HALF_ANG2RAD = 3.14159265358979323846f / 180.0f;
	glm::vec3 camPos; // camera position
	glm::vec3 front, right, up; // the camera referential
	float width, height, ratio;
	double tang;
	float sphereFactorX, sphereFactorY;

	Frustum(void) {}; // Private constructor to allow only one instance
	Frustum(Frustum const&); // Private constructor to prevent copies
	void operator=(Frustum const&); // Private constructor to prevent assignments
public:

	~Frustum();
   /*Returns the pointer to the unique instance of the render loop class.*/
	static Frustum* Frustum::getInstance()
	{
		static Frustum instance;// lazy singleton, instantiated on first use
		return &instance;
	}

	void setCamInternals(float angle, int width, int height);
	void setCamDef(glm::vec3 camPos, glm::vec3 front, glm::vec3 up, glm::vec3 right);
	int pointInFrustum(glm::vec3 p); // Returns -1 if the point is on the outside, 0 if on, and 1 if in the frustum
	int sphereInFrustum(glm::vec3 p, float radius); // Returns -1 if the sphere is on the outside, 0 if on, and 1 if in the frustum
	int boxInFrustum(); // Returns -1 if the box is on the outside, 0 if on, and 1 if in the frustum
};



