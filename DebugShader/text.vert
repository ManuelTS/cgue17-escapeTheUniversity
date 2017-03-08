#version 430 core

layout (location = 0) in vec4 positionAndTC; // position.xy and texture coordinates.zw, usage in text.cpp

layout (location = 0) out vec2 texCoord; // Usage in text.frag

layout (location = 0) uniform mat4 transform; // Usage in text.cpp

void main()
{
    gl_Position = transform * vec4(positionAndTC.xy, 0.0, 1.0);
    texCoord = positionAndTC.zw;
}
