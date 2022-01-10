
#version 150

#define FLAT        0
#define GOURAUD     1
#define PHONG       2   

// In Arguments
in  vec3 vPosition;
in  vec3 vNormal;

// Out Arguments
out vec3 vertexColor;
out vec3 vertexNormal;
out vec3 fragmentPosition;
flat out int shadow;
flat out vec3 polygonColor;


// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int shadow_type;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float La;
uniform float Ld;
uniform float Ls;


void main()
{
    vertexColor = color;
    fragmentPosition = vec3(modelViewMatrix * modelMatrix * vec4(vPosition, 1.0));
    vertexNormal = mat3(modelViewMatrix * normalMatrix ) * vNormal;
    if(shadow_type == PHONG)
    {
        shadow = PHONG;
    }
    else // GOURAUD or FLAT
    {
        vec3 normalizeNormal = normalize(vertexNormal);
        vec3 lightDirection = normalize(lightPosition - fragmentPosition);
	    vec3 reflectDir = reflect(-lightDirection, normalizeNormal); 
	    vec3 viewDirection = normalize(0 - fragmentPosition);

	    vec3 ambient = La * lightColor;
	    vec3 diffuse =  Ld * max(dot(normalizeNormal, lightDirection), 0.0) * lightColor; 
        vec3 specular  = Ls * pow(max(dot(viewDirection, reflectDir), 0.0), 2) * lightColor; 
	    vec3 result = (ambient + diffuse + specular) * vertexColor;
        if(shadow_type == GOURAUD)
        {
            shadow = GOURAUD;
	        vertexColor = result;
        }
        else // FLAT
        {
            polygonColor = result;
            shadow = FLAT;
        }
    }

    gl_Position = projectionMatrix * vec4(fragmentPosition,1.0);

}