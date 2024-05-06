#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor; // Use a color instead of a texture

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor; // Output a color to the fragment shader

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);
    ourColor = aColor; // Pass the color to the fragment shader
}
