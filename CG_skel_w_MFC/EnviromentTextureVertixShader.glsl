#version 150

in vec3 vPosition;
out vec3 TexCoords;

uniform mat4 modelViewMatrix; //view;
uniform mat4 projectionMatrix; //projection;

void main()
{
    TexCoords = vPosition;
    vec4 pos = projectionMatrix * modelViewMatrix * vec4(vPosition, 1.0);
    gl_Position = pos.xyww;
}  