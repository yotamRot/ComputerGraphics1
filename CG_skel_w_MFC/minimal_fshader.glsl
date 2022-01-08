
#version 150

// In Arguments
in  vec3 vertexColor;
in  vec3 vertexNormal;
in  vec3 fragmentPosition;

// Out Arguments
out vec4 fColor;

// Uniforms declaration
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition;
uniform float La;
uniform float Ld;
uniform float Ls;
//uniform float Alpha;



void main() 
{ 

    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
	vec3 reflectDir = reflect(-lightDirection, vertexNormal); 
	vec3 viewDirection = normalize(viewPosition - fragmentPosition);


	vec3 ambient = La * lightColor;
	vec3 diffuse = Ld * max(dot(vertexNormal, lightPosition), 0.0) * lightColor; 
    vec3 specular = Ls * pow(max(dot(viewDirection, reflectDir), 0.0), 1) * lightColor; 


	vec3 result = (ambient + diffuse + specular) * vertexColor;
	fColor = vec4(result,1.0);
} 
