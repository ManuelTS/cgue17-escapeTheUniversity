#version 430 core

layout(location = 0) in vec2 texCoord; // Usage in text.vert

layout (location = 4) uniform float scale; // Usage in text.cpp
layout (binding = 0) uniform sampler2D ourTexture; // Usage in text.cpp

void main()
{
      float smoothing = 0.25 / ( 1 * scale );
      
	  if(smoothing > 0.49) 
		smoothing = 0.49;

      const float distance2Outline = texture(ourTexture, texCoord).a;
      const float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance2Outline);
      gl_FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
