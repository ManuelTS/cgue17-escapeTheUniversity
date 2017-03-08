#version 430 core
const vec4 verts[4] = vec4[4](vec4(-1.0, -1.0, 0.5, 1.0), 
                              vec4( 1.0, -1.0, 0.5, 1.0),
                              vec4(-1.0,  1.0, 0.5, 1.0),
                              vec4( 1.0,  1.0, 0.5, 1.0)); //Arraysize in GBuffer#rederQuad()

void main(void)
{
    gl_Position = verts[gl_VertexID];
}
