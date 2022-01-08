#version 150


// In Arguments
in  vec3 vPosition;

// Out Arguments
out vec3 lightVertexColor;

// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;//model;
uniform mat4 modelViewMatrix;//view;
uniform mat4 projectionMatrix;//projection;

void main()
{
	gl_Position = projectionMatrix * modelViewMatrix * modelMatrix * vec4(vPosition,1.0);
	lightVertexColor = color;
}