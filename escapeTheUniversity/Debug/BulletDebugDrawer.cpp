#include <vector>
#include "../Model/Mesh/Mesh.hpp"
#include "BulletDebugDrawer.hpp"

using namespace std;

BulletDebugDrawer::BulletDebugDrawer(){
	m = new Mesh();
}

BulletDebugDrawer::~BulletDebugDrawer(){
	delete m;
}

void BulletDebugDrawer::BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{ // Line gathering from http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=10071
	Mesh::Vertex fromV;
	Mesh::Vertex toV;

	fromV.position = glm::vec3(from.getX(), from.getY(), from.getZ());
	toV.position = glm::vec3(to.getX(), to.getY(), to.getZ());

	m->data.push_back(fromV);
	m->data.push_back(toV);
	m->indices.push_back(index++);
	m->indices.push_back(index++);
	m->materials.push_back(glm::vec4(0, 0, 0, 1));
	m->materials.push_back(glm::vec4(0, 0, 0, 1));
}

void BulletDebugDrawer::draw()
{
	m->draw(GL_LINES);
	m->data.clear();
	m->indices.clear();
	m->materials.clear();
}

void BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
{}

void BulletDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color){
}

void BulletDebugDrawer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha){
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString){
}

void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString){
}

void BulletDebugDrawer::setDebugMode(int db)
{
	debugMode = db;
}
