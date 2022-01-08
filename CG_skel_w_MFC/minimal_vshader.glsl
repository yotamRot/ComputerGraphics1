#version 150

// In Arguments
in  vec4 vPosition;


// Out Arguments
out vec3 vertexColor;


// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{
    gl_Position = projectionMatrix  * modelViewMatrix * modelMatrix * vPosition;
    vertexColor = color;
}