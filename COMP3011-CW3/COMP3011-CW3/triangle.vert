#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord; // Add this line

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out vec2 TexCoord; // Add this line

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);
    ourColor = aColor; // Pass the color to the fragment shader
    TexCoord = aTexCoord; // Pass the texture coordinate to the fragment shader
}
