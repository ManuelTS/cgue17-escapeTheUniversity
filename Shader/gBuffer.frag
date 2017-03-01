#version 430 core

layout (binding = 0) uniform sampler2D textureDiffuse; // Usage in: Mesh.cpp draw();

layout (location = 0) in vec3 fragmentPosition; // Usage in: gBuffer.vert out
layout (location = 1) in vec3 normalVector;     // Usage in: gBuffer.vert out, normalized
layout (location = 2) in vec2 texCoords;        // Usage in: gBuffer.vert out
layout (location = 3) in vec4 materialDiffuseShininess; // Usage in: gBuffer.vert out

layout (location = 0) out uvec4 gColorNormal;        // Usage in: deferredShading.frag in
layout (location = 1) out vec4 gPositionAndShininess;// Usage in: deferredShading.frag in, normalized

// Check in master texture manaagment, google for uniform color in def. shading

void main()
{
	vec3 color = texture(textureDiffuse, texCoords).rgb; 
	gColorNormal.x = packHalf2x16(color.xy);
	gColorNormal.y = packHalf2x16(vec2(color.z,normalVector.x));
	gColorNormal.z = packHalf2x16(normalVector.yz);
	//gColorNormal.w = uint(0.0); // Unused

	gPositionAndShininess.xyz = fragmentPosition;
	gPositionAndShininess.w = materialDiffuseShininess.a; //texture(textureSpecular, texCoords).r; // Specular texture NOT IMPLEMENTED, // materialDiffuseShininess.rgb is unused!
}
