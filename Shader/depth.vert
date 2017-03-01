#version 430 core

layout (location = 0) in vec3 position; // Usage in: Mesh.cpp link();

layout (location = 0)  uniform mat4 model;	      // Usage in: Node.hpp
layout (location = 4)  uniform mat4 view;	      // Usage in: RenderLoop.cpp init(); before loop
layout (location = 8) uniform mat4 projection;   // Usage in: RenderLoop.cpp init(); before loop

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
