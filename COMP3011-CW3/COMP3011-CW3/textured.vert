#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTex;
layout(location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float texScale; // For repeating scaled textures
uniform mat4 projectedLightSpaceMatrix; 

out vec2 TexCoord;
out vec3 Normal; // Output the normal to the fragment shader
out vec3 FragPos; // Output the fragment position to the fragment shader
out vec4 FragPosProjectedLightSpace; // Output the fragment position in light space

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);
    TexCoord = aTex.xy * texScale; // Apply the texture scaling
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform the normal
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate the fragment position
    FragPosProjectedLightSpace = projectedLightSpaceMatrix * vec4(FragPos, 1.0); // Calculate the fragment position in light space
}