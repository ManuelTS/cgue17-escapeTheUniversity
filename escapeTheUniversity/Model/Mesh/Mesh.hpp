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
		glm::uvec4 boneIndices = glm::uvec4(0); // Bone indices used for the correct boneMatrix in the shader
		glm::vec4 boneWeights = glm::vec4(0); // The maximal four bone weights effecting this vertex read from corresponding aiBone in "assimpBoneNode" to deliver weights along with their vertex with VAO into the shader
	};

	/*Contains all texture ids, names, and paths.*/
	struct Texture{
		unsigned int id;
		std::string name;
		std::string path;
	};
	
	std::vector<Vertex> vertices; // Contains all vertices and their information
	aiNode* assimpBoneNode = nullptr; // Node as Bone referrence for the animator class
	unsigned int meshIndex; // Mesh this mesh index of assimpBoneNode
	std::vector<unsigned int> indices; // Contains all indices of this mesh
	std::vector<glm::vec4> materials; // Materials, rbg material values, a shininess
	std::vector<Texture> textures; // Textures

	ModelNode* modelNode; // Is the node this mesh belongs to
	const static unsigned int positionsLocation = 0; // In gBuffer.vert, stencil.vert, stencilDebug.vert, deferredShading.vert

	Mesh();
	~Mesh();
	
	void link(); // Creates and links the VAO for this mesh
	void draw(unsigned int drawMode, bool shadow); // drawMode is the openGl draw mode of glDrawElements, shadow is true if data is writting into shado shaders, false if in normal shaders
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
	const unsigned int normalsLocation = 1; // In gBuffer.vert
	const unsigned int uvLocation = 2; // UVs, In gBuffer.vert
	const unsigned int boneIndicesLocation = 3; // Bone indices inside all the bonde matrices on the shader
	const unsigned int boneWeightLocation = 4; // boneWeight, In gBuffer.vert
	const unsigned int materialLocation = 5; // In gBuffer.vert
	const unsigned int boneMatricesLocation = 16; // In gBuffer.vert

	const unsigned int MAX_BONE_NUMER = 60; // Max bone number

	RenderLoop* rl = RenderLoop::getInstance(); // to set the drawn trianlges

	void transmitBoneMatrix();// Transmits the bone matrices to the vertex shader
	glm::mat4 transposeAssimpMatrix2GLMColumnMajor(aiMatrix4x4 mat); // Convertes the argument to the return type, delete the return pointer!
};