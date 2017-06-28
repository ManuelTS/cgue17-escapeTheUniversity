#pragma once
#define GLM_FORCE_RADIANS // Use this for radiant calculation, the GLM one does not work!

#include <GLM\glm.hpp>

// Calculates if a point or shepre is in, out, or intersects the view frustum with the radar approach
class Frustum
{
private:
	glm::vec3 camPos; // camera position
	glm::vec3 front, right, up; // the camera referential, see camera.cpp
	float ratio, sphereFactorX, sphereFactorY;;
	double tang;

	Frustum(void) {}; // Private constructor to allow only one instance
	Frustum(Frustum const&); // Private constructor to prevent copies
	void operator=(Frustum const&); // Private constructor to prevent assignments
public:
	const float nearD = 0.01f; // Distance from the camera to the near plane
	const float farD = 100.0f; // Distance from the camera to the far plane
	const float actionRadius = 6.0f;
	~Frustum();
   /*Returns the pointer to the unique instance of the render loop class.*/
	static Frustum* Frustum::getInstance()
	{
		static Frustum instance;// lazy singleton, instantiated on first use
		return &instance;
	}

	void setCamInternals(float angle, int width, int height);
	void setCamDef(glm::vec3 camPos, glm::vec3 front, glm::vec3 right, glm::vec3 up);
	int pointInFrustum(glm::vec3 p); // Returns -1 if the point is on the outside, 0 if on the frustum borders, and 1 if in the frustum
	int sphereInFrustum(const glm::vec3 sphereCenter, const float radius); // Returns -1 if the sphere is on the outside, 0 if on the frustum borders, and 1 if in the frustum
	int Frustum::inActionRadius(const glm::vec3 actionTarget); //returns -1 if it is in actionRadius, and 1 if it is in actionRadius (simplyfied)
	float degreesToRadians(float degrees); // Argument in degrees, returns the radians for it
};



