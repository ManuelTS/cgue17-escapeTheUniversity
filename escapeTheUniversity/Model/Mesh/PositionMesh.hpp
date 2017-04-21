#pragma once

#include "..\..\RenderLoop.hpp"
#include "Mesh.hpp"
#include <GLM\glm.hpp>
#include <vector>

/*Only contains the positions of meshes.*/
class PositionMesh : public Mesh{
private:
	vector<glm::vec4> data; // Positions of this mesh
public:
	PositionMesh(std::vector<unsigned int>_indices, std::vector<glm::vec4> _data);
	~PositionMesh();

	void draw() override; // Draws or rather writes only the positions into the shader
};