#include <iostream>
#include <vector>
#include "../Model/Mesh/Mesh.hpp"
#include "../Text.hpp"
#include "BulletDebugDrawer.hpp"

using namespace std;

BulletDebugDrawer::BulletDebugDrawer(){
	container = new Mesh(); // Empty container mesh, no VAO generated only for data container
}

BulletDebugDrawer::~BulletDebugDrawer(){
	delete container;
}

// Called when btIDebugDraw::DBG_DrawWireframe is set
void BulletDebugDrawer::BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{	// Line gathering from http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=10071
	Mesh::Vertex fromV;
	Mesh::Vertex toV;

	fromV.position = glm::vec3(from.getX(), from.getY(), from.getZ());
	toV.position = glm::vec3(to.getX(), to.getY(), to.getZ());

	container->vertices.push_back(fromV);
	container->vertices.push_back(toV);
	container->indices.push_back(index++);
	container->indices.push_back(index++);
	container->materials.push_back(glm::vec4(1, 0, 0, 1)); // Draw lines red
	container->materials.push_back(glm::vec4(1, 0, 0, 1));
}

void BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
{// Line gathering from http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=10071
	Mesh::Vertex fromV;
	Mesh::Vertex toV;

	fromV.position = glm::vec3(from.getX(), from.getY(), from.getZ());
	toV.position = glm::vec3(to.getX(), to.getY(), to.getZ());

	container->vertices.push_back(fromV);
	container->vertices.push_back(toV);
	container->indices.push_back(index++);
	container->indices.push_back(index++);
	container->materials.push_back(glm::vec4(fromColor.getX(), fromColor.getY(), fromColor.getZ(), 1));
	container->materials.push_back(glm::vec4(toColor.getX(), toColor.getY(), toColor.getZ(), 1));
}

void BulletDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color){
	int i = 0;
}

void BulletDebugDrawer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha){
	int i = 0;
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){
	int i = 0;
}

// Called when btIDebugDraw::DBG_DrawAabb (overflow in AABB)
void BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
	cout << warningString << endl;
	Text::getInstance()->bulletDebugMessage(warningString ? warningString : "Empty bullet debug message.");
}

void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString){
	int i = 0;
}

void BulletDebugDrawer::setDebugMode(int db)
{
	debugMode = db;
}

void BulletDebugDrawer::draw()
{
	Mesh* drawer = new Mesh(container->indices, container->vertices, container->textures, container->materials); // Generate and link VAO only here since it can change a in size
	drawer->draw(GL_LINES);
	delete drawer;
	container->clear();
	index = 0;
}