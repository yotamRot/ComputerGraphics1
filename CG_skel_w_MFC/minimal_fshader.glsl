
#version 150


// In Arguments
in  vec4 vertexColor;


// Out Arguments

out vec4 fColor;


// Uniforms declaration

uniform vec4 lightColor;



void main() 
{ 
   fColor = lightColor * vertexColor;
} 
