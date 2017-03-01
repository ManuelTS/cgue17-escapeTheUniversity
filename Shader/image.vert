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
}

