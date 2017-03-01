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
}
