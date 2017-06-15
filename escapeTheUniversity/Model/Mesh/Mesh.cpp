#include "Mesh.hpp"
#include <GLM\gtc\type_ptr.hpp>
#include "..\ModelLoader.hpp"
#include "..\..\Debug\Debugger.hpp"
#include "..\..\Debug\MemoryLeakTracker.h"

using namespace std;

Mesh::~Mesh(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1,&VAO);
	clear();
}

Mesh::Mesh()
{
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
}

void Mesh::link()
{
	//Link
	glGenVertexArrays(1, &VAO); // Generate and setup normal VAO and VBO
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO); // Multiple VAOS can refer to the same element buffer

	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionsLocation);
	glVertexAttribPointer(positionsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(normalsLocation);
	glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	glEnableVertexAttribArray(boneIndicesLocation); // Must be bound here, only possible without zeros if an other shader is used
	glVertexAttribIPointer(boneIndicesLocation, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIndices));
	glEnableVertexAttribArray(boneWeightLocation);
	glVertexAttribPointer(boneWeightLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

	glGenBuffers(1, &materialVBO); // RGB = optional color, if all are not zero the texture is unused, only vec4.a for shininess
	glBindBuffer(GL_ARRAY_BUFFER, materialVBO);
	glBufferData(GL_ARRAY_BUFFER, materials.size() * sizeof(materials[0]), &materials[0], GL_STATIC_DRAW); // Play with last param 4 performance
	glVertexAttribPointer(materialLocation, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0); // Unbind VAO first!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

glm::mat4 Mesh::transposeAssimpMatrix2GLMColumnMajor(aiMatrix4x4 mat)
{
	// Column and row major orders: https://en.wikipedia.org/wiki/Transpose
	// OpenGL calls for matrices with GL_FALSE does not transpostion https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glUniform.xml
	// OpenGL default order is column major, https://stackoverflow.com/questions/29191344/matrix-order-in-skeletal-animation-using-assimp
	// GLM matrices are column major, http://glm.g-truc.net/0.9.8/glm-0.9.8.pdf
	glm::mat4 transposed; 

	transposed[0][0] = mat.a1;	transposed[0][1] = mat.b1;	transposed[0][2] = mat.c1;	transposed[0][3] = mat.d1;
	transposed[1][0] = mat.a2;	transposed[1][1] = mat.b2;	transposed[1][2] = mat.c2;	transposed[1][3] = mat.d2;
	transposed[2][0] = mat.a3;	transposed[2][1] = mat.b3;	transposed[2][2] = mat.c3;	transposed[2][3] = mat.d3;
	transposed[3][0] = mat.a4;	transposed[3][1] = mat.b4;	transposed[3][2] = mat.c4;	transposed[3][3] = mat.d4;

	return transposed;
}

void Mesh::transmitBoneMatrix()
{
	if (assimpBoneNode != nullptr)
	{
		glm::mat4 boneMatrices[60]; //MAX_BONE_NUMER; cannot use a constant here... -.-
		const std::vector<aiMatrix4x4>& vBoneMatrices = ModelLoader::getInstance()->animator->GetBoneMatrices(assimpBoneNode, meshIndex);

		for (unsigned int matrixIndex = 0; matrixIndex < vBoneMatrices.size(); matrixIndex++)
			boneMatrices[matrixIndex] = transposeAssimpMatrix2GLMColumnMajor(vBoneMatrices[matrixIndex]);

		glUniformMatrix4fv(boneMatricesLocation, MAX_BONE_NUMER, GL_FALSE, glm::value_ptr(boneMatrices[0]));
	}
}

/*Draws this mesh*/
void Mesh::draw(unsigned int drawMode, bool shadow)
{		
	if (shadow)
	{// Shadow draw with shadow shaders
		glBindVertexArray(VAO); // Only the vertex positions are programmed (and used) in the shader

		glDisableVertexAttribArray(normalsLocation); // Disable all other bound vertex attributes 
		glDisableVertexAttribArray(uvLocation);
		glDisableVertexAttribArray(boneIndicesLocation); 
		glDisableVertexAttribArray(boneWeightLocation);
		glDisableVertexAttribArray(materialLocation);

		glDrawElements(drawMode, indices.size(), GL_UNSIGNED_INT, 0); // Draw

		glEnableVertexAttribArray(normalsLocation); // Enable all other bound vertex attributes for further "normal" rendering
		glEnableVertexAttribArray(uvLocation);
		glEnableVertexAttribArray(boneIndicesLocation);
		glEnableVertexAttribArray(boneWeightLocation);
		glEnableVertexAttribArray(materialLocation);

		glBindVertexArray(0);
	}
	else
	{ // Normal draw
		for (unsigned int i = 0; i < textures.size() && i < maxTextureUnits; i++)// Bind textures
		{
			if (textures[i].name == "textureDiffuse") //Spectrail in frag shader anyways
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
			}
			/*else if (textures[i].name == "textureSpecular") // Spectral texture not used in frag shader anyways
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glUniform1i(textureSpecularLocation, i);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
			}*/
		}	

		transmitBoneMatrix();

		glBindVertexArray(VAO);

		if (rl->fps)
			rl->drawnTriangles += vertices.size() / 3;

		glDrawElements(drawMode, indices.size(), GL_UNSIGNED_INT, 0); // Draw
		glBindVertexArray(0);

		for (unsigned int i = 0; i < textures.size(); i++)// Unbind textures
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void Mesh::clear() {
	indices.clear();
	vertices.clear();
	textures.clear();
	materials.clear();
}

vector<int>* Mesh::getAllIndices()
{
	return (vector<int>*)&indices;
}

vector<float>* Mesh::getAllVertices() {
	unsigned int size = vertices.size();

	if (size > 0)
	{
		vector<float>* temp = new vector<float>();


		for (unsigned int j = 0; j < size; j++) // Go through all mesh vertices
		{
			const glm::vec3 position = vertices.at(j).position; // Get all the vertices
			temp->push_back(position.x);
			temp->push_back(position.y);
			temp->push_back(position.z);
		}

		
		return temp;
	}

	return nullptr;
}