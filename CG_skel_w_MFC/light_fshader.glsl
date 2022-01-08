#version 150


// In Arguments
in  vec3 lightVertexColor;


// Out Arguments
out vec4 FragColor;

// Uniforms declaration

void main()
{
    FragColor =  vec4(lightVertexColor,1);
}