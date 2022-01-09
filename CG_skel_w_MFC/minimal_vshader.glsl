#version 150

// In Arguments
in  vec3 vPosition;
in  vec3 vNormal;

// Out Arguments
out vec3 vertexColor;
out vec3 vertexNormal;
out vec3 fragmentPosition;


// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{

    vertexColor = color;
    fragmentPosition = vec3(modelViewMatrix * modelMatrix * vec4(vPosition, 1.0));
    vertexNormal = mat3(modelViewMatrix * normalMatrix ) * vNormal;
    

    gl_Position = projectionMatrix * modelViewMatrix * vec4(fragmentPosition,1.0);
}