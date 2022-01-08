
#version 150

// In Arguments
in  vec3 vertexColor;

// Out Arguments
out vec4 fColor;

// Uniforms declaration
uniform vec3 lightColor;
uniform float La;



void main() 
{ 
	vec3 ambient = La * lightColor;
	vec3 result = ambient * vertexColor;
	fColor = vec4(result,1);
} 
