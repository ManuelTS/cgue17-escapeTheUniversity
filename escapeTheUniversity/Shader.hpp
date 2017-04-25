#pragma once

#include <string>
#include <GL\glew.h>

class Shader
{
public:
	// gBufferShader location constants
	const int viewLocation = 8; // gBuffer.vert
	const int projectionLocation = 12; // gBuffer.vert
	// deferredShader location constants
	const int viewPositionLocation = 0; // defferredShader.frag

	unsigned int programHandle; // Shader handle

	Shader(const char* shader);
	~Shader();

	void useProgram() const;
private:
	unsigned int vertexHandle;
	unsigned int fragmentHandle;

	void loadShader(const char* shader, GLenum shaderType, unsigned int& handle);
	void link();
	void check(unsigned int handle, const std::string& name);

	const char* TEXT_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec4 positionAndTC; // screen position.xy and texture coordinates.zw, usage in text.cpp

	layout (location = 0) out vec2 texCoord; // Usage in text.frag

	layout (location = 0) uniform mat4 transform; // Usage in text.cpp

	void main()
	{
	  gl_Position = transform * vec4(positionAndTC.xy, 0.0, 1.0);
	  texCoord = positionAndTC.zw;
	})glsl";
	const char* TEXT_FRAG = R"glsl(
	#version 430 core

	layout(location = 0) in vec2 texCoord; // Usage in text.vert

	layout (location = 4) uniform vec4 colorScale; // Usage in text.cpp, xyz is color, a is scale
	layout (binding = 0) uniform sampler2D ourTexture; // Usage in text.cpp

	void main()
	{
		float smoothing = 0.25 / ( 1 * colorScale.a );

		if(smoothing > 0.49)
		smoothing = 0.49;

		const float distance2Outline = texture(ourTexture, texCoord).a;
		const float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance2Outline);
		gl_FragColor = vec4(colorScale.xyz, alpha);
	})glsl";
	const char* IMAGE_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec2 position;// Usage in: RenderLoop.cpp displayLoadingScreen();
	layout (location = 1) in vec3 color;   // Usage in: RenderLoop.cpp displayLoadingScreen();
	layout (location = 2) in vec2 texCoord;// Usage in: RenderLoop.cpp displayLoadingScreen();

	layout (location = 0) out vec3 fragmentColor;    // Usage in: image.frag
	layout (location = 1) out vec2 fragmentTexCoord; // Usage in: image.frag

	void main()
	{
		fragmentColor = color;
		fragmentTexCoord = texCoord;
		gl_Position = vec4(position, 0.0, 1.0);
	})glsl";
	const char* IMAGE_FRAG = R"glsl(
	#version 430 core

	layout (location = 0) in vec3 color;
	layout (location = 1) in vec2 texCoord;

	layout (location = 0) uniform sampler2D tex;

	void main()
	{
		gl_FragColor  = texture(tex, texCoord) * vec4(color, 1.0);
	})glsl";
	const char* GBUFFER_VERT = R"glsl(
	#version 430 core

	layout (location = 0)  uniform mat4 model;	      // Usage in: Node.hpp
	layout (location = 4)  uniform mat4 inverseModel; // Usage in: Node.hpp
	layout (location = 8)  uniform mat4 view;	      // Usage in: RenderLoop.cpp init(); before loop
	layout (location = 12) uniform mat4 projection;   // Usage in: RenderLoop.cpp init(); before loop

	// This four must be the same as DEFERRED_SHADING_STENCIL_VERT
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
		texCoords.x = tc.x;                         // Forward uv texel coordinates to the fragment shader
		texCoords.y = 1 - tc.y;                     // Forward uv texel coordinates to the fragment shader, TODO finde cause and solution to this flipped y coords wordaround
		normalVector = mat3(inverseModel) * normal; // Forward normals to fragment shader
		materialDiffuseShininess = material;        // rgb unused
	})glsl";

	const char* GBUFFER_FRAG = R"glsl(
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
		gPositionAndShininess.w = materialDiffuseShininess.a; // Specular texture NOT IMPLEMENTED, // materialDiffuseShininess.rgb is unused!
	})glsl";
	const char* DEFERRED_SHADING_VERT = R"glsl(
	#version 430 core
	const vec4 verts[4] = vec4[4](vec4(-1.0, -1.0, 0.5, 1.0), 
								  vec4( 1.0, -1.0, 0.5, 1.0),
								  vec4(-1.0,  1.0, 0.5, 1.0),
								  vec4( 1.0,  1.0, 0.5, 1.0)); //Arraysize in GBuffer#rederQuad()

	void main(void)
	{
		gl_Position = verts[gl_VertexID];
	})glsl";
	const char* DEFERRED_SHADING_FRAG = R"glsl(
	#version 430 core

	layout (location = 0) uniform vec3 viewPosition; // from RenderLoop.cpp#renderloop

	layout (binding = 0) uniform usampler2D colorAndNormalTex;      // From gBuffer.frag, attachment0
	layout (binding = 1) uniform sampler2D positionAndShininessTex; // From gBuffer.frag, attachment1

	struct LightStruct 
	{ // Same as LightNode.hpp#Light
		vec4 position; //w = 0 pointLight, 1 directional
		vec4 ambient; 
		vec4 diffuse; 
		vec4 specular;
		vec4 shiConLinQua; // x = shininess, y = constant attentuation, z = linear attentuation, w = quadratic attentuation value
	};

	layout (std140, binding = 2, index = 0) uniform LightBlock 
	{
		LightStruct light;
	} l; // Workaround, direct struct blockbinding only in openGL 4.5

	// Blinn Phong light
	vec3 calculateLight(vec3 diffuse, float specular, vec3 norm, vec3 fragmentPosition, vec3 viewDirection)
	{
		//  Calculate ambientColor
		vec3 ambientColor = diffuse * 0.01;

		// Calculate diffuseColor 
		vec3 lightPosition = l.light.position.xyz;
		vec3 lightDirection= normalize(lightPosition - fragmentPosition);
		float diffuseImpact = max(dot(norm, lightDirection), 0.0); // Max for no negative values
		vec3 diffuseColor = l.light.diffuse.rgb * (diffuseImpact * diffuse);

		// Calculate spectralColor
		vec3 halfwayDir = normalize(lightDirection + viewDirection); // Blinn Phong
		float spec = pow(max(dot(norm, halfwayDir), 0.0), l.light.shiConLinQua.x);
		vec3 specularColor = l.light.specular.rgb * (spec * specular);

		//Calculate attenuation
		float lightFragDist = length(lightPosition - fragmentPosition);
		float attenuation = 1.0 / (l.light.shiConLinQua.y + l.light.shiConLinQua.z * lightFragDist + l.light.shiConLinQua.w * (lightFragDist * lightFragDist));

		diffuseColor *= attenuation;
		specularColor *= attenuation;

		// Calculate Final color	
		return ambientColor + diffuseColor + specularColor;
	}

	void main()
	{
		//Unpack GBuffer
		uvec4 gColorAndNormal = texelFetch(colorAndNormalTex, ivec2(gl_FragCoord.xy), 0); //gColorAndNormal.w; // unused!
		vec4 gPositionAndShininess = texelFetch(positionAndShininessTex, ivec2(gl_FragCoord.xy), 0);
		vec2 temp = unpackHalf2x16(gColorAndNormal.y);

		vec3 diffuse = vec3(unpackHalf2x16(gColorAndNormal.x), temp.x);
		float specular = gPositionAndShininess.w;
		vec3 norm = normalize(vec3(temp.y, unpackHalf2x16(gColorAndNormal.z)));
		vec3 fragmentPosition = gPositionAndShininess.xyz;
		vec3 viewDirection = normalize(viewPosition - fragmentPosition);

		//Calculate light
		vec3 color = calculateLight(diffuse, specular, norm, fragmentPosition, viewDirection);
	
		color = vec(0.0f, 1.0f, 0.0f); // Without light, just get color on screen, that means depth or stencil is wrong

		gl_FragColor = vec4(color, 1.0);
	})glsl";
	const char* DEFERRED_SHADING_STENCIL_VERT = R"glsl(
	#version 430 core

	// This four must be the same as in GBUFFER_VERT
	layout (location = 0) in vec3 position; // Usage in: Mesh.cpp link(); // Of a point of the light sphere, used in .cpp, w is unused and must be one
	layout (location = 1) in vec3 normal;   // Usage in: Mesh.cpp link();
	layout (location = 2) in vec2 tc;       // Usage in: Mesh.cpp link();
	layout (location = 3) in vec4 material; // Usage in: Mesh.cpp link();, rgb unused

	layout (location = 0) uniform mat4 model;      // translated, scaled sphere model of a light, used in gBuffer.hpp
	layout (location = 4) uniform mat4 view;       // of the camera, used in gBuffer.hpp
	layout (location = 8) uniform mat4 projection; // to see what is on the screen, used in gBuffer.hpp

	void main()
	{          
		gl_Position = projection * view * model * vec4(position, 1.0f); // model * position = worldPosition
	})glsl";
	const char* DEFERRED_SHADING_STENCIL_FRAG = R"glsl(
	#version 430 core

	void main()	{})glsl";
	const char* DEPTH_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec3 position; // Usage in: Mesh.cpp link();

	layout (location = 0)  uniform mat4 model;	      // Usage in: Node.hpp
	layout (location = 4)  uniform mat4 view;	      // Usage in: RenderLoop.cpp init(); before loop
	layout (location = 8) uniform mat4 projection;   // Usage in: RenderLoop.cpp init(); before loop

	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0f);
	})glsl";
	const char* DEPTH_FRAG = R"glsl(
	#version 430 core

	const float near = 1.0; 
	const float far  = 100.0; 
  
	float linearizeDepth(float depth) 
	{
		const float z = depth * 2.0 - 1.0; // Back to NDC 
		return (2.0 * near * far) / (far + near - z * (far - near));	
	}

	void main()
	{             
		const float depth = linearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
		gl_FragColor = vec4(vec3(depth), 1.0f);
	})glsl";
};
