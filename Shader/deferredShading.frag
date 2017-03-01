#version 430 core

layout (location = 0) uniform vec3 viewPosition; // from RenderLoop.cpp#renderloop
layout (binding = 0) uniform usampler2D colorAndNormalTex;      // From gBuffer.frag, attachment0
layout (binding = 1) uniform sampler2D positionAndShininessTex; // From gBuffer.frag, attachment1#

// Deeply understand location, binding, and index of sampler2D

struct Light
{ // Same as LightNode.hpp#Light
	vec4 position; 
	vec4 ambient; 
	vec4 diffuse; 
	vec4 specular;
	vec4 shiConLinQua; // x = shininess, y = constant attentuation, z = linear attentuation, w = quadratic attentuation value
};

const int MAX_LIGHTS = 10; // Correlates with ModelLoader.hpp#MAX_LIGHTS
layout (std140, binding = 2, index = 0) uniform LightBlock  // Used in RendererLoop#drawLights
{
	Light light[MAX_LIGHTS];
}lights;

vec3 calculateLight(Light currentLight, vec3 diffuse, float specular, vec3 norm, vec3 fragmentPosition, vec3 viewDirection)
{
	//  Calculate ambientColor
	vec3 ambientColor = diffuse * 0.01;

	// Calculate diffuseColor 
	vec3 lightPosition = currentLight.position.rgb;
	vec3 lightDirection= normalize(lightPosition - fragmentPosition);
	float diffuseImpact = max(dot(norm, lightDirection), 0.0); // Max for no negative values
	vec3 diffuseColor = currentLight.diffuse.rgb * (diffuseImpact * diffuse);

	// Calculate spectralColor
	vec3 halfwayDir = normalize(lightDirection + viewDirection); // Blinn Phong
	float spec = pow(max(dot(norm, halfwayDir), 0.0), currentLight.shiConLinQua.x);
	vec3 specularColor = currentLight.specular.rgb * (spec * specular);

	if(currentLight.position.w == 0.0) // Point light
	{  //Nothing in here, it is okay
	}
	else if(currentLight.position.w == 1.0) // Directional light
	{   //Calculate attenuation
		float lightFragDist = length(lightPosition - fragmentPosition);
		float attenuation = 1.0 / (currentLight.shiConLinQua.y + currentLight.shiConLinQua.z * lightFragDist + currentLight.shiConLinQua.w * (lightFragDist * lightFragDist));

		diffuseColor *= attenuation;
		specularColor *= attenuation;
	}

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

	// Calculate lights
	vec3 color = vec3(0.0, 0.0, 0.0);

	// Point Light
	// ... maybe will come
	// Directional Light
	for(int i = 0; i < lights.light.length(); i++)
		if(lights.light[i].diffuse.x > 0.0 || lights.light[i].diffuse.y > 0.0 || lights.light[i].diffuse.z > 0.0) // Draw only non-empty lights
			color += calculateLight(lights.light[i], diffuse, specular, norm, fragmentPosition, viewDirection);
	// Spot Light
	// ... maybe will come

	gl_FragColor = vec4(color, 1.0);
}