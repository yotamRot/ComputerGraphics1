#version 150


in  vec4 vPosition;

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{
    gl_Position = projectionMatrix  * modelViewMatrix * modelMatrix * vPosition;
}