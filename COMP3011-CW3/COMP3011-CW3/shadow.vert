#version 450 core

layout (location = 0) in vec4 vPos;

uniform mat4 model;
uniform mat4 projectedLightSpaceMatrix;

void main()
{
	gl_Position = projectedLightSpaceMatrix * model * vPos;
}