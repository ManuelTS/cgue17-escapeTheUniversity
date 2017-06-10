#pragma once

#include "..\..\RenderLoop.hpp"
#include <GLM\glm.hpp>
#include <vector>
#include <string>
#include <assimp/scene.h>

/*Contains all single data sets for a meshes in a system of array style.*/
class Mesh{
public:
	/*Contains all vertex positions, normals, and text coordinates.*/
	struct Vertex
	{
		glm::vec3 position; // Position
		glm::vec3 normal; // Normal
		glm::vec2 texCoords; // TexCoords
	};

	struct Bone
	{
		glm::vec4 weight;
		glm::uvec4 index;
	};

	/*Contains all texture ids, names, and paths.*/
	struct Texture{
		unsigned int id;
		std::string name;
		std::string path;
	};
	
	std::vector<Vertex> vertices; // Contains all vertices and their information
	std::vector<Bone> bones; // Contains all bones and their weights and indices
	std::vector<unsigned int> indices; // Contains all indices of this mesh
	std::vector<glm::vec4> materials; // Materials, rbg material values, a shininess
	std::vector<Texture> textures; // Textures

	Mesh();
	~Mesh();
	
	void link(); // Creates and links the VAO for this mesh
	void draw(unsigned int drawMode = GL_TRIANGLES);
	void clear(); // Clears all vectors
	vector<int>* getAllIndices(); // Returns a pointer to all vertex indices of all meshes in this node. Dont' delete vector after usage!
	vector<float>* getAllVertices(); // Returns a pointer to all vertices of all meshes in this node. Delete vector after usage!
private:
	// Shared handles
	unsigned int EBO;
	// Normal handles
	unsigned int VAO;
	unsigned int VBO;
	unsigned int materialVBO;

	int maxTextureUnits = 0;
	const unsigned int positionsLocation = 0; // In gBuffer.vert and stencil.vert
	const unsigned int normalsLocation = 1; // In gBuffer.vert
	const unsigned int uvLocation = 2; // UVs, In gBuffer.vert
	const unsigned int materialLocation = 3; // In gBuffer.vert

	RenderLoop* rl = RenderLoop::getInstance(); // to set the drawn trianlges
};