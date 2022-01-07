
#version 150

// In arguments
// in  vec2 texCoord;
in  vec4 vertex_color;	// input from vertex shader

// Out arguments
out vec4 fColor;

//uniform sampler2D texture;

void main() 
{ 
   // fColor = textureLod( texture, texCoord, 0 );
   fColor = vertex_color;
} 

