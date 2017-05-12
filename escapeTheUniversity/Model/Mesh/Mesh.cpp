#include "Mesh.hpp"
#include "..\..\Shader.hpp"
#include "..\..\Debug\MemoryLeakTracker.h"
#include <iostream>

using namespace std;

Mesh::~Mesh(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1,&VAO);
}

/*Links the VBOs, indices, positions, normals, textCoords (UVs), textureIds, -names and -paths together in one VAO.*/
Mesh::Mesh(vector<unsigned int> _indices, vector<Vertex> _data, vector<Texture> _textures, vector<glm::vec4> _materials, bool generateStencilVAO) : indices(_indices), data(_data), textures(_textures), materials(_materials)
{
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	//Link
	glGenBuffers(1, &EBO); // Multiple VAOS can refer to the same element buffer
	glGenVertexArrays(1, &VAO); // Generate and setup normal VAO and VBO
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	partitialSetup(VAO, VBO);
	glEnableVertexAttribArray(normalsLocation);
	glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));

	glGenBuffers(1, &materialVBO); // RGB unused, only vec4.a for shininess
	glBindBuffer(GL_ARRAY_BUFFER, materialVBO);
	glBufferData(GL_ARRAY_BUFFER, materials.size() * sizeof(materials[0]), &materials[0], GL_STATIC_DRAW); // Play with last param 4 performance
	glVertexAttribPointer(materialLocation, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glBindVertexArray(0); // Unbind VAO first!
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (generateStencilVAO)
	{
		glGenVertexArrays(1, &stencilVAO); // Generate VAO and VBO
		glGenBuffers(1, &stencilVBO);
		
		partitialSetup(stencilVAO, stencilVBO);

		glBindVertexArray(0); // Unbind VAO first!
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::partitialSetup(unsigned int currentVAO, unsigned int currentVBO) 
{
	glBindVertexArray(currentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vertex), &data[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionsLocation);
	glVertexAttribPointer(positionsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
}

/*Draws this mesh*/
void Mesh::draw()
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
			rl->drawnTriangles += data.size() / 3;

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // Draw
		glBindVertexArray(0);

		for (unsigned int i = 0; i < textures.size(); i++)// Unbind textures
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
}

void Mesh::stencilDraw()
{
	glBindVertexArray(stencilVAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
