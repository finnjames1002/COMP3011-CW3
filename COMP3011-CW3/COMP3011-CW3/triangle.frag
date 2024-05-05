#version 330 core
in vec3 ourColor; // receive color from the vertex shader

out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0f); // use the incoming color to color the triangle
}