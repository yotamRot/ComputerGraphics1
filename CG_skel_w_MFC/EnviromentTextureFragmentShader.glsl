#version 150


out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube enviromentTexture;

void main()
{    
    FragColor = texture(enviromentTexture, TexCoords);
}