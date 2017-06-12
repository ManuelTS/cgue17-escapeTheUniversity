#pragma once

#include <string>
#include <GL\glew.h>

class Shader
{
public:
	// gBufferShader location constants
	const int viewLocation = 8; // Matrix, gBuffer.vert
	const int projectionLocation = 12; // Matrix, gBuffer.vert
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
		gl_FragColor = vec4(colorScale.rgb, alpha);
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

	layout (location = 0)  uniform mat4 model;	          // Usage in: ModelNode.hpp#draw()
	layout (location = 4)  uniform mat4 inverseModel;     // Usage in: ModelNode.hpp#draw()
	layout (location = 8)  uniform mat4 view;	          // Usage in: RenderLoop.cpp#doDeferredShading()
	layout (location = 12) uniform mat4 projection;       // Usage in: RenderLoop.cpp#doDeferredShading()
	layout (location = 16) uniform mat4 boneMatrices[60]; // Usage in: Mesh.cpp#transmitBoneMatrix(), and max bones allowed

	layout (location = 0) in vec3 position;    // Usage in: Mesh.cpp link();
	layout (location = 1) in vec3 normal;      // Usage in: Mesh.cpp link();
	layout (location = 2) in vec2 tc;          // Usage in: Mesh.cpp link();
	layout (location = 3) in uvec4 boneIndices;// Usage in: Mesh.cpp link();
	layout (location = 4) in vec4 boneWeights; // Usage in: Mesh.cpp link();
	layout (location = 5) in vec4 material;    // Usage in: Mesh.cpp link();, // rgb = optional color, if all are not zero the texture is unused, a = shininess value

	layout (location = 0) out vec3 fragmentPosition; // Usage in: gBuffer.frag in
	layout (location = 1) out vec3 normalVector;     // Usage in: gBuffer.frag in
	layout (location = 2) out vec2 texCoords;        // Usage in: gBuffer.frag in
	layout (location = 3) out vec4 materialDiffuseShininess; // Usage in: gBuffer.frag in

	void main()
	{
		vec4 worldPosition = vec4(position, 1);

		/*if(boneWeights.x > 0 || boneWeights.y > 0 || boneWeights.z > 0 || boneWeights.w > 0)
		{
			vec4 usedBoneWeights = boneWeights;
			usedBoneWeights.w = 1.0 - dot(boneWeights.xyz, vec3(1.0, 1.0, 1.0));
			
			mat4 transformMatrix = usedBoneWeights.x * boneMatrices[int(boneIndices.x)]; // TODO: test if int cast is necessary
			transformMatrix += usedBoneWeights.y * boneMatrices[int(boneIndices.y)];
			transformMatrix += usedBoneWeights.z * boneMatrices[int(boneIndices.z)];
			transformMatrix += usedBoneWeights.w * boneMatrices[int(boneIndices.w)];

			worldPosition = model * transformMatrix * worldPosition;
		}
		else*/
			worldPosition  = model * worldPosition;

		fragmentPosition = worldPosition.xyz;
		gl_Position = projection * view * worldPosition;

		texCoords = tc;                             // Forward uv texel coordinates to the fragment shader
		normalVector = mat3(inverseModel) * normal; // Forward normals to fragment shader
		materialDiffuseShininess = material;        // rgb = optional color, if all are not zero the texture is unused, a = shininess value
	})glsl";
	const char* GBUFFER_FRAG = R"glsl(
	#version 430 core

	layout (binding = 0) uniform sampler2D textureDiffuse; // Usage in: Mesh.cpp draw();

	layout (location = 0) in vec3 fragmentPosition; // Usage in: gBuffer.vert out
	layout (location = 1) in vec3 normalVector;     // Usage in: gBuffer.vert out, normalized
	layout (location = 2) in vec2 texCoords;        // Usage in: gBuffer.vert out
	layout (location = 3) in vec4 materialDiffuseShininess; // Usage in: gBuffer.vert out, rgb = optional color, if one is not zero the texture is unused, a = shininess value

	layout (location = 0) out uvec4 gColorNormal;        // Usage in: deferredShading.frag in
	layout (location = 1) out vec4 gPositionAndShininess;// Usage in: deferredShading.frag in, normalized

	// Check in master texture manaagment, google for uniform color in def. shading

	void main()
	{
		vec3 color = materialDiffuseShininess.rgb;
		uint textureLookup = 0; // Handle to know in the deferred frag shader if only a color value or a texture is used

		if(materialDiffuseShininess.r == 0 && materialDiffuseShininess.g == 0 && materialDiffuseShininess.b == 0) // rgb = optional color, if one is not zero the texture is unused, a = shininess 
		{
			color = texture(textureDiffuse, texCoords).rgb;
			textureLookup = 1; // Handle to know in the deferred frag shader if only a color value or a texture is used
		}
		
		gColorNormal.x = packHalf2x16(color.xy);
		gColorNormal.y = packHalf2x16(vec2(color.z,normalVector.x));
		gColorNormal.z = packHalf2x16(normalVector.yz);
		gColorNormal.w = textureLookup; // Handle to know in the deferred frag shader if only a color value or a texture is used

		gPositionAndShininess.xyz = fragmentPosition;
		gPositionAndShininess.w = materialDiffuseShininess.a; // a is the material shininess value
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
		vec4 position;     // xyz = world coord position of light, w = flag if it should be drawn or not 1.0 = true = yes, otherwise false = no.
		vec4 diffuse;      // rgb = diffuse light, a = ambient coefficient
		vec4 specular;     // w unused
		vec4 shiConLinQua; // x = shininess, y = constant attentuation, z = linear attentuation, w = quadratic attentuation value
	};

	const int LIGHT_NUMBER = 10; // Correlates with ModelLoader.hpp#LIGHT_NUMBER
	layout (std140, binding = 2, index = 0) uniform LightBlock 
	{
		LightStruct light[LIGHT_NUMBER];
	} l; // Workaround, direct struct blockbinding only in openGL 4.5

	const float RIM_POWER = 10.0f; // Rim light power

	// Calculate Rim Light
	vec3 calculateRim(LightStruct light, vec3 normal, vec3 viewDirection)
	{
		// Calculate the rim factor
		float rimFactor = 1.0 - dot(normal, viewDirection);

		// Constrain it to the range 0 to 1 using a smoothstep function
		rimFactor = smoothstep(0.0, 1.0, rimFactor);

		// Raise it to the rim exponent
		rimFactor = pow(rimFactor, RIM_POWER);

		// Finally, multiply it by the rim color which is here simply the light diffuse color
		return rimFactor * light.diffuse.rgb;
	}

	// Blinn Phong light
	vec3 calculateLight(LightStruct light, vec3 diffuse, float specular, vec3 norm, vec3 fragmentPosition, vec3 viewDirection)
	{
		//  Calculate ambientColor
		vec3 ambientColor = diffuse * light.diffuse.a;

		// Calculate diffuseColor 
		vec3 lightPosition = light.position.xyz;
		vec3 lightDirection = normalize(lightPosition - fragmentPosition);
		float diffuseImpact = max(dot(norm, lightDirection), 0.0); // Max for no negative values
		vec3 diffuseColor = light.diffuse.rgb * (diffuseImpact * diffuse);

		// Calculate spectralColor
		vec3 halfwayDir = normalize(lightDirection + viewDirection); // Blinn Phong
		float spec = pow(max(dot(norm, halfwayDir), 0.0), light.shiConLinQua.x);
		vec3 specularColor = light.specular.rgb * (spec * specular);

		//Calculate attenuation
		float lightFragDist = length(lightPosition - fragmentPosition);
		float attenuation = 1.0 / (light.shiConLinQua.y + light.shiConLinQua.z * lightFragDist + light.shiConLinQua.w * (lightFragDist * lightFragDist));

		diffuseColor *= attenuation;
		specularColor *= attenuation;

		// Calculate Final color	
		return ambientColor + diffuseColor + specularColor;// + calculateRim(norm, viewDirection);
	}

	void main()
	{
		//Unpack GBuffer
		uvec4 gColorAndNormal = texelFetch(colorAndNormalTex, ivec2(gl_FragCoord.xy), 0);
		vec4 gPositionAndShininess = texelFetch(positionAndShininessTex, ivec2(gl_FragCoord.xy), 0);
		vec2 temp = unpackHalf2x16(gColorAndNormal.y);

		vec3 diffuse = vec3(unpackHalf2x16(gColorAndNormal.x), temp.x);
		float directColor = gColorAndNormal.w; // Handle to know in the deferred frag shader if only a color value or a texture is used
		
		if(directColor == 0) // Handle to know in the deferred frag shader if only a color value or a texture is used
			gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		else
		{
			float specular = gPositionAndShininess.w;
			vec3 norm = normalize(vec3(temp.y, unpackHalf2x16(gColorAndNormal.z)));
			vec3 fragmentPosition = gPositionAndShininess.xyz;
			vec3 viewDirection = normalize(viewPosition - fragmentPosition);

			//Calculate light
			vec3 color = vec3(0.0f);

			for(int i = 0; i < LIGHT_NUMBER; i++)
				if(l.light[i].position.w >= -1.0f)
					color += calculateLight(l.light[i], diffuse, specular, norm, fragmentPosition, viewDirection);

			gl_FragColor = vec4(color, 1.0f);
		}
	})glsl";
	const char* DEPTH_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec3 position; // Usage in: Mesh.cpp link(); // Of a point of the light sphere, used in .cpp, w is unused and must be one
	layout (location = 1) in vec3 normal;   // Usage in: Mesh.cpp link();
	layout (location = 2) in vec2 tc;       // Usage in: Mesh.cpp link();
	layout (location = 3) in vec4 material; // Usage in: Mesh.cpp link();, rgb unused

	layout (location = 0)  uniform mat4 model;	     // Usage in: Node.hpp
	layout (location = 4)  uniform mat4 view;	     // Usage in: RenderLoop.cpp init(); before loop
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
