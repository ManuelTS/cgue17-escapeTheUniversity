#include "PositionMesh.hpp"

using namespace std;

PositionMesh::~PositionMesh(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1,&VAO);
}

/*Links the VBOs, indices, positions, normals, textCoords (UVs), textureIds, -names and -paths together in one VAO.*/
PositionMesh::PositionMesh(vector<unsigned int> _indices, vector<glm::vec4> _data) : data(_data)
{
	indices = _indices;
	//Link
	glGenVertexArrays(1, &VAO); // Bind
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), &data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionsLocation);
	glVertexAttribPointer(positionsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);

	glBindVertexArray(0); // Unbind VAO first!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/*Draws this mesh*/
void PositionMesh::draw()
{		
	glBindVertexArray(VAO);

	if (rl->fps)
		rl->drawnTriangles += data.size() / 3;

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // Draw
	glBindVertexArray(0);
}
