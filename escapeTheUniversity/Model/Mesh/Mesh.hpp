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
	
	std::vector<Vertex> data;

	/*Contains all texture ids, names, and paths.*/
	struct Texture{
		unsigned int id;
		std::string name;
		std::string path;
	};

	Mesh() {};
	Mesh(std::vector<unsigned int>_indices, std::vector<Vertex> _data, std::vector<Texture> _textures, std::vector<glm::vec4> _materials);
	~Mesh();
	
	void draw();
protected:
	unsigned int VAO;
	unsigned int EBO;
	unsigned int VBO;
	unsigned int materialVBO;
	RenderLoop* rl = RenderLoop::getInstance();

	std::vector<unsigned int> indices;
	std::vector<Texture> textures; // Textures
	std::vector<glm::vec4> materials; // Materials, rbg material values, a shininess

	int maxTextureUnits = 0;
	const unsigned int positionsLocation = 0; // In gBuffer.vert
	const unsigned int normalsLocation = 1; // In gBuffer.vert
	const unsigned int uvLocation = 2; // UVs, In gBuffer.vert
	const unsigned int materialLocation = 3; // In gBuffer.vert
};