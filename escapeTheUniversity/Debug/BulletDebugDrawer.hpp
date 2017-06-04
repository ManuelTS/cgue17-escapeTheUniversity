#pragma once
#include "Bullet/btBulletDynamicsCommon.h"

class BulletDebugDrawer : public btIDebugDraw
{
private:
	int debugMode = 0; // Debugt mode, default 0 = no debugging: http://bulletphysics.org/Bullet/BulletFull/classbtIDebugDraw.html
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int index; // Vertex index array
	Mesh* container; // Mesh used to draw all the primitives coming from bullet
public:
		BulletDebugDrawer();
		~BulletDebugDrawer();
		void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;
		void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor) override;
		void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;
		void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) override;
		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
		void reportErrorWarning(const char* warningString) override;
		void draw3dText(const btVector3& location, const char* textString) override;
		void setDebugMode(int debugMode) override;
		int  getDebugMode() const override
		{
			return debugMode;
		}
		void draw();//Draws the received bullet draw data
};

