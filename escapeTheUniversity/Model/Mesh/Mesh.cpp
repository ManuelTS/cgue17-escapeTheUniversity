#include "Mesh.hpp"
#include "..\..\Shader.hpp"
#include "..\..\Debug\MemoryLeakTracker.h"
#include <iostream>

using namespace std;

Mesh::~Mesh(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1,&VAO);
	clear();
}

/*Links the VBOs, indices, positions, normals, textCoords (UVs), textureIds, -names and -paths together in one VAO.*/
Mesh::Mesh(vector<unsigned int> _indices, vector<Vertex> _vertices, vector<Texture> _textures, vector<glm::vec4> _materials) : indices(_indices), vertices(_vertices), textures(_textures), materials(_materials)
{
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
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
	glVertexAttribPointer(positionsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(normalsLocation);
	glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));

	glGenBuffers(1, &materialVBO); // RGB = optional color, if all are not zero the texture is unused, only vec4.a for shininess
	glBindBuffer(GL_ARRAY_BUFFER, materialVBO);
	glBufferData(GL_ARRAY_BUFFER, materials.size() * sizeof(materials[0]), &materials[0], GL_STATIC_DRAW); // Play with last param 4 performance
	glVertexAttribPointer(materialLocation, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glBindVertexArray(0); // Unbind VAO first!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*Draws this mesh*/
void Mesh::draw(unsigned int drawMode)
{		
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