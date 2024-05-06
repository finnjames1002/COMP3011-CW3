#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor; // Use a color instead of a texture
layout(location = 2) in vec3 aNormal; // Add a normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor; // Output a color to the fragment shader
out vec3 Normal; // Output the normal to the fragment shader
out vec3 FragPos; // Output the fragment position to the fragment shader

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);
    ourColor = aColor; // Pass the color to the fragment shader
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform the normal
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate the fragment position
}
