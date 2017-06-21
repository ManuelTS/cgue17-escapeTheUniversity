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
	// shadowShader location constants
	const unsigned int SHADOW_LIGHT_POSITION_LOCATION = 0; // shadowShader.vert
	const unsigned int SHADOW_MODEL_MATRIX_LOCATION = 0; // shadowShader.vert
	const unsigned int SHADOW_LIGHT_SPACE_MATRIX_LOCATION = 1; // in the shadowShader.vert

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

	layout (location = 0) in vec3 position;    // Usage in: Mesh.cpp link(); correlates in location value with shadow.vert
	layout (location = 1) in vec3 normal;      // Usage in: Mesh.cpp link();
	layout (location = 2) in vec2 tc;          // Usage in: Mesh.cpp link();
	layout (location = 3) in uvec4 boneIndices;// Usage in: Mesh.cpp link();
	layout (location = 4) in vec4 boneWeights; // Usage in: Mesh.cpp link();
	layout (location = 5) in vec4 material;    // Usage in: Mesh.cpp link();, // rgb = unused, a = shininess value

	layout (location = 0) out vec3 fragmentPosition; // Usage in: gBuffer.frag in
	layout (location = 1) out vec3 normalVector;     // Usage in: gBuffer.frag in
	layout (location = 2) out vec2 texCoords;        // Usage in: gBuffer.frag in
	layout (location = 3) out vec4 materialDiffuseShininess; // Usage in: gBuffer.frag in

	void main()
	{
		vec4 worldPosition = vec4(position, 1);
		vec3 usedNormal = normal;

		if(boneWeights.x > 0 || boneWeights.y > 0 || boneWeights.z > 0 || boneWeights.w > 0)
		{ // Calculate the skinning matrix for vertices which have bones, maximum are four bone influences
			vec4 usedBoneWeights = boneWeights;
			usedBoneWeights.w = 1.0 - dot(boneWeights.xyz, vec3(1.0, 1.0, 1.0));
			
			mat4 boneMatrix = usedBoneWeights.x * boneMatrices[int(boneIndices.x)]; // TODO: test if int cast is necessary // Bone transformation matrix
			boneMatrix += usedBoneWeights.y * boneMatrices[int(boneIndices.y)];
			boneMatrix += usedBoneWeights.z * boneMatrices[int(boneIndices.z)];
			boneMatrix += usedBoneWeights.w * boneMatrices[int(boneIndices.w)];

			worldPosition = model * boneMatrix * worldPosition;

			usedNormal = vec3(boneMatrix * vec4(normal, 0));
		}
		else // Normal world position calculation
			worldPosition  = model * worldPosition;

		fragmentPosition = worldPosition.xyz;
		gl_Position = projection * view * worldPosition;

		texCoords = tc;                             // Forward uv texel coordinates to the fragment shader
		normalVector = mat3(inverseModel) * usedNormal; // Forward normals to fragment shader
		materialDiffuseShininess = material;        // rgb = optional color, if all are not zero the texture is unused, a = shininess value
	})glsl";
	const char* GBUFFER_FRAG = R"glsl(
	#version 430 core

	layout (binding = 0) uniform sampler2D textureDiffuse; // Usage in: Mesh.cpp draw();

	layout (location = 256) uniform vec4 flags; // x = flag for debugging to render bullet wireframe with value 1, used in RenderLoop#doDeferredShading and BulletDebugDrawer#draw

	layout (location = 0) in vec3 fragmentPosition; // Usage in: gBuffer.vert out
	layout (location = 1) in vec3 normalVector;     // Usage in: gBuffer.vert out, normalized
	layout (location = 2) in vec2 texCoords;        // Usage in: gBuffer.vert out
	layout (location = 3) in vec4 materialDiffuseShininess; // Usage in: gBuffer.vert out, rgb = optional color, if one is not zero the texture is unused, a = shininess value

	layout (location = 0) out uvec4 gColorNormal; // Usage in: deferredShading.frag in
	layout (location = 1) out vec4 gPosition;     // Usage in: deferredShading.frag in, normalized

	void main()
	{
		vec3 color = texture(textureDiffuse, texCoords).rgb;

		if(flags.x > 0) // Flag for debugging to render bullet wireframe with value 1
			color = materialDiffuseShininess.rgb;
		
		gColorNormal.x = packHalf2x16(color.xy);
		gColorNormal.y = packHalf2x16(vec2(color.z,normalVector.x));
		gColorNormal.z = packHalf2x16(normalVector.yz);
		gColorNormal.w = packHalf2x16(vec2(materialDiffuseShininess.a, 0.0f)); // x = shininess value, y unused

		gPosition.xyz = fragmentPosition;
		gPosition.w = flags.x; // Flag for debugging to render bullet wireframe with value 1
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

	layout (location = 1) uniform mat4 lightSpaceMatrix; // Light space transformation matrix that transforms each world-space vector into the space as visible from the light source

	layout (binding = 0) uniform usampler2D colorAndNormalTex; // From gBuffer.frag, attachment0
	layout (binding = 1) uniform sampler2D positionTex;        // From gBuffer.frag, attachment1
	layout (binding = 2) uniform sampler2D depthMap;           // From shadowMapping, depth map, shadowMapping#bindTexture()

	struct LightStruct 
	{ // Same as LightNode.hpp#Light
		vec4 position;     // xyz = world coord position of light, w = unused
		vec4 diffuse;      // rgb = diffuse light, a = ambient coefficient
		vec4 shiConLinQua; // x = shininess, y = constant attentuation, z = linear attentuation, w = quadratic attentuation value
	};

	layout (std140, binding = 2, index = 0) uniform LightBlock 
	{
		LightStruct light;
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
	
	float calculateShadow(vec3 fragmentPosition, vec3 lightDirection, vec3 normal)
	{
		vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragmentPosition, 1.0f); // fragmentPosition is in world coords
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;            // perform perspective divide
	    projCoords = projCoords * 0.5 + 0.5;                                      // Because the depth from the depth map is in the range [0,1] and we also want to use projCoords to sample from the depth map so we transform the NDC coordinates to the range [0,1]
	    float closestDepth = texture(depthMap, projCoords.xy).r;                  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
		float currentDepth = projCoords.z;                                        // To get the current depth at the fragment we simply retrieve the projected vector's z coordinate which equals the depth of the fragment from the light's perspective.
		float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);      // Fight shadow acne, Here we have a maximum bias of 0.05 and a minimum of 0.005 based on the surface's normal and light direction. This way surfaces like the floor that are almost perpendicular to the light source get a small bias, while surfaces like the cube's side-faces get a much larger bias. 
		float shadow = 0.0f;												      // PCF, percentage-closer filtering. The idea is to sample more than once from the depth map, each time with slightly different texture coordinates. 
		vec2 texelSize = 1.0 / textureSize(depthMap, 0);
		
		for(int x = -1; x <= 1; x++)
			for(int y = -1; y <= 1; y++)
			{
	            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
		        shadow += currentDepth - bias > pcfDepth  ? 0.0 : 1.0;        
			}    
		
		shadow /= 9.0f;
	
		if(projCoords.z > 1.0) // Avoid over sampling: Keep the shadow at 1.0 when outside the far_plane region of the light's frustum.
			shadow = 1.0;

		return shadow; // 0.x in shadow, 1 not in shadow
	}

	// Blinn Phong light
	vec3 calculateLight(LightStruct light, vec3 diffuse, float materialShininess, vec3 norm, vec3 fragmentPosition, vec3 viewDirection)
	{
		//  Calculate ambientColor
		vec3 ambientColor = diffuse * light.diffuse.a;

		// Calculate diffuseColor 
		vec3 lightPosition = light.position.xyz;
		vec3 lightDirection = normalize(lightPosition - fragmentPosition);
		float diffuseImpact = max(dot(norm, lightDirection), 0.0); // Max for no negative values
		vec3 diffuseColor = light.diffuse.rgb * diffuseImpact * diffuse;

		// Calculate spectralColor
		vec3 halfwayDir = normalize(lightDirection + viewDirection); // Blinn Phong
		float spec = pow(max(dot(norm, halfwayDir), 0.0), light.shiConLinQua.x);
		vec3 specularColor = light.diffuse.rgb * spec * materialShininess;

		//Calculate attenuation
		float lightFragDist = length(lightPosition - fragmentPosition);
		float attenuation = 1.0 / (light.shiConLinQua.y + light.shiConLinQua.z * lightFragDist + light.shiConLinQua.w * (lightFragDist * lightFragDist));

		diffuseColor *= attenuation;
		specularColor *= attenuation;

		//Calculate shadow
	    float shadow = calculateShadow(fragmentPosition, lightDirection, norm);  

		// Calculate Final color	
		return ambientColor + ((diffuseColor + specularColor) * shadow);// + calculateRim(norm, viewDirection);
	}

	void main()
	{ //Unpack GBuffer
		uvec4 gColorAndNormal = texelFetch(colorAndNormalTex, ivec2(gl_FragCoord.xy), 0);
		vec4 gPosition = texelFetch(positionTex, ivec2(gl_FragCoord.xy), 0); // flag to display color immediately without light calculations
		vec2 temp = unpackHalf2x16(gColorAndNormal.y);

		vec3 diffuse = vec3(unpackHalf2x16(gColorAndNormal.x), temp.x);

		if(gPosition.w == 1.0f)
			gl_FragColor = vec4(diffuse, 1.0f);
		else
		{
			vec2 shininess = unpackHalf2x16(gColorAndNormal.w); // x = shininess color value, y = unused

			float materialShininess = shininess.x;
			vec3 norm = normalize(vec3(temp.y, unpackHalf2x16(gColorAndNormal.z)));
			vec3 fragmentPosition = gPosition.xyz;
			vec3 viewDirection = normalize(viewPosition - fragmentPosition);

			//Calculate color with light
			vec3 color = calculateLight(l.light, diffuse, materialShininess, norm, fragmentPosition, viewDirection);

			gl_FragColor = vec4(color, 1.0f);	
		}
	})glsl";
	const char* STENCIL_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec3 position;              // of the rendered vertex, correlates in location value with gbuffer.vert

	layout (location = 0)  uniform mat4 model;	      // Usage in: RenderLoop.cpp#doDeferredShading Stencil Pass
	layout (location = 4)  uniform mat4 view;	      // Usage in: RenderLoop.cpp#doDeferredShading Stencil Pass
	layout (location = 8) uniform mat4 projection;   // Usage in: RenderLoop.cpp#doDeferredShading Stencil Pass

	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0f);
	})glsl";
	const char* STENCIL_FRAG = R"glsl( // Empty shader, color is not needed
	#version 430 core
	
	void main()
	{
	})glsl";
	const char* SHADOW_VERT = R"glsl(
	#version 430 core

	layout (location = 0) in vec3 position;              // of the rendered vertex, correlates in location value with gbuffer.vert

	layout (location = 0) uniform mat4 model;            // Model matrix of the rendered object
	layout (location = 1) uniform mat4 lightSpaceMatrix; // Light space transformation matrix that transforms each world-space vector into the space as visible from the light source

	void main()
	{
		gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
	})glsl";
	const char* SHADOW_FRAG = R"glsl( // Empty shader vor normal depthMap rendering
	#version 430 core
	
	void main()
	{
	})glsl";
	const char* SHADOW_DEBUG_VERT = R"glsl(
	#version 430 core
	
	layout (location = 0) in vec3 position;    // in debugger.cpp#renderShadowMap of the pixel on screen [0,1]
	layout (location = 1) in vec2 inTextCoords; // in debugger.cpp#renderShadowMap of the depth map [0,1] rendered on screen

	layout (location = 0) out vec2 outTexCoords; // of the depth map [0,1] rendered on screen for the fragment shader

	void main()
	{
		outTexCoords = inTextCoords;
		gl_Position = vec4(position, 1.0);
	})glsl";
	const char* SHADOW_DEBUG_FRAG = R"glsl(
	#version 430 core

	layout (location = 0) in vec2 texCoords; // of the depth map [0,1] rendered on screen

	layout (location = 0) uniform float near_plane; // in debugger.cpp#renderShadowMap
	layout (location = 1) uniform float far_plane;  // in debugger.cpp#renderShadowMap

	layout (binding = 0) uniform sampler2D depthMap; // in debugger.cpp#renderShadowMap, depthMap rendered with shaodw_vert shader

	float linearizeDepth(float depth)
	{
		float z = depth * 2.0 - 1.0; // Back to NDC 
		return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
	}

	void main()
	{             
		float depthValue = texture(depthMap, texCoords).r; // .r is correct got it tested with a little depth visualization
		gl_FragColor = vec4(vec3(linearizeDepth(depthValue) / far_plane), 1.0); // perspective
		//gl_FragColor = vec4(vec3(depthValue), 1.0); // orthographic
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
