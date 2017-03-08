#version 430 core

layout (location = 0) in vec3 color;
layout (location = 1) in vec2 texCoord;

layout (location = 0) uniform sampler2D tex;

void main()
{
	gl_FragColor  = texture(tex, texCoord) * vec4(color, 1.0);
}