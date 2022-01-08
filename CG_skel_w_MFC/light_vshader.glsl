#version 150


// In Arguments
in  vec4 vPosition;

// Out Arguments
out vec3 lightVertexColor;

// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;//model;
uniform mat4 modelViewMatrix;//view;
uniform mat4 projectionMatrix;//projection;

void main()
{
	gl_Position = projectionMatrix * modelViewMatrix * modelMatrix * vPosition;
	lightVertexColor = color;
}