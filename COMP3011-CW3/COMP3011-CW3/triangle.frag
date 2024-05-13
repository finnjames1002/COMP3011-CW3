#version 330 core

in vec3 ourColor; // receive color from the vertex shader
in vec2 TexCoord; // texture coordinates

out vec4 FragColor;

uniform sampler2D texture_diffuse; // diffuse map

void main()
{
    vec3 texColor = texture(texture_diffuse, TexCoord).rgb; // Sample the texture
    FragColor = vec4(ourColor * texColor, 1.0) * 2.5; // Multiply the texture color with the vertex color
}
