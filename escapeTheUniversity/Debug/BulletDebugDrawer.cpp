#include <iostream>
#include <vector>
#include "BulletDebugDrawer.hpp"
#include "../Text.hpp"
#include "../Model/Node/ModelNode.hpp"


using namespace std;

// btIDebugDraw::DrawFeaturesText no method call
BulletDebugDrawer::BulletDebugDrawer()
{
	container = new Mesh(); // Empty container mesh, no VAO generated only for data container
	modelMatrix = glm::mat4();
	inverseModelMatrix = glm::inverseTranspose(modelMatrix);
}

BulletDebugDrawer::~BulletDebugDrawer()
{
	delete container;
}

// Called when btIDebugDraw::DBG_DrawWireframe is set
void BulletDebugDrawer::BulletDebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{	// Line gathering from http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=10071
	Mesh::Vertex fromV;
	Mesh::Vertex toV;

	fromV.position = glm::vec3(from.getX(), from.getY(), from.getZ());
	toV.position = glm::vec3(to.getX(), to.getY(), to.getZ());
	fromV.texCoords = glm::vec2(0, 0);
	toV.texCoords = glm::vec2(0, 0);
	fromV.normal = fromV.position;
	toV.normal = toV.position;

	container->vertices.push_back(fromV);
	container->vertices.push_back(toV);
	container->indices.push_back(index++);
	container->indices.push_back(index++);
	container->materials.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	container->materials.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
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

// btIDebugDraw::DBG_DrawContactPoints
void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	int i = 0;
}

// Called when btIDebugDraw::DBG_DrawAabb (overflow in AABB)
void BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
	cerr << warningString << endl;
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
	if(!container->indices.empty() || !container->vertices.empty())
	{ 
		Mesh* drawer = new Mesh(); // Generate and link VAO only here since it can change a in size

		drawer->materials = container->materials;
		drawer->indices = container->indices;
		drawer->vertices = container->vertices;

		drawer->link();

		glUniformMatrix4fv(ModelNode::modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(ModelNode::inverseModelLocation, 1, GL_FALSE, glm::value_ptr(inverseModelMatrix));
		glUniform4f(ModelNode::debugFlagLocation, 1, 0, 0, 0); // x = flag for debugging to render bullet wireframe with value 1

		glDepthFunc(GL_LEQUAL);

		drawer->draw(GL_LINES, false);
		delete drawer;
		container->clear();
		index = 0;
	}
	else
	{
		if (index == 0) // Display it only once
		{
			cerr << "No indices or vertices are set from bullet." << endl;
			Text::getInstance()->bulletDebugMessage("No indices or vertices are set from bullet.");
		}
		index = 1;
	}
}