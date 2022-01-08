#version 150


// In Arguments
in  vec4 lightVertexColor;


// Out Arguments
out vec4 FragColor;

// Uniforms declaration

void main()
{
    FragColor =  lightVertexColor;
}