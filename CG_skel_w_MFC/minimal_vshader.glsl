#version 150

// In Arguments
in  vec4 vPosition;
in  vec3 vNormal;

// Out Arguments
out vec3 vertexColor;
out vec3 vertexNormal;
out vec3 fragmentPosition;


// Uniforms declaration
uniform vec3 color;
uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{
    gl_Position = projectionMatrix  * modelViewMatrix * modelMatrix * vPosition;
    vertexColor = color;
    vertexNormal = normalize(mat3(transpose(inverse(modelMatrix))) * vNormal);
    fragmentPosition = vec3(modelMatrix * vPosition);
}