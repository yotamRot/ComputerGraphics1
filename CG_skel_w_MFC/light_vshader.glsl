#version 150


// In Arguments
in  vec3 vPosition;
uniform int isNormal;

// Out Arguments
out vec3 lightVertexColor;

// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;//model;
uniform mat4 normalMatrix;//model;
uniform mat4 modelViewMatrix;//view;
uniform mat4 projectionMatrix;//projection;


void main()
{
	vec3 fragmentPosition ;
	if (isNormal == 1)
	{
		fragmentPosition = vec3(modelViewMatrix * modelMatrix * vec4(vPosition, 1.0));
	}
	else
	{
		fragmentPosition = vec3(modelViewMatrix * modelMatrix * vec4(vPosition, 1.0));
	}
	gl_Position = projectionMatrix *  vec4(fragmentPosition,1.0);
	lightVertexColor = color;
}