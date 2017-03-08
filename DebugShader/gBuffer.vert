#version 430 core

layout (location = 0)  uniform mat4 model;	      // Usage in: Node.hpp
layout (location = 4)  uniform mat4 inverseModel; // Usage in: Node.hpp
layout (location = 8)  uniform mat4 view;	      // Usage in: RenderLoop.cpp init(); before loop
layout (location = 12) uniform mat4 projection;   // Usage in: RenderLoop.cpp init(); before loop

layout (location = 0) in vec3 position; // Usage in: Mesh.cpp link();
layout (location = 1) in vec3 normal;   // Usage in: Mesh.cpp link();
layout (location = 2) in vec2 tc;       // Usage in: Mesh.cpp link();
layout (location = 3) in vec4 material; // Usage in: Mesh.cpp link();, rgb unused

layout (location = 0) out vec3 fragmentPosition; // Usage in: gBuffer.frag in
layout (location = 1) out vec3 normalVector;     // Usage in: gBuffer.frag in
layout (location = 2) out vec2 texCoords;        // Usage in: gBuffer.frag in
layout (location = 3) out vec4 materialDiffuseShininess; // Usage in: gBuffer.frag in

void main()
{
	vec4 worldPosition = vec4(model * vec4(position, 1.0));
	fragmentPosition = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
    texCoords.x = tc.x; // Forward uv texel coordinates to the fragment shader
    texCoords.y = 1 - tc.y; // Forward uv texel coordinates to the fragment shader, TODO finde cause and solution to this flipped y coords wordaround
	normalVector = mat3(inverseModel) * normal; // Forward normals to fragment shader
	materialDiffuseShininess = material; // rgb unused
}
