#version 150

// In arguments

// in  vec2 vTexCoord;
in  vec4 vPosition;

// Out arguments

// out vec2 texCoord;
out vec4 vertex_color;

//Uniforms
uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
// uniform mat4 projectionMatrix;
//uniform mat4 viewMatrix;
//uniform mat3 normalMatrix;

void main()
{
    gl_Position = modelViewMatrix * modelMatrix * vPosition;
    // texCoord = vTexCoord;
    vertex_color = vec4(color,1.0); 
}
