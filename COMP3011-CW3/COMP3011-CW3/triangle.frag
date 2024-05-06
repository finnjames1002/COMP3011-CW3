#version 330 core

in vec3 ourColor; // receive color from the vertex shader
in vec3 Normal; // Receive the normal from the vertex shader
in vec3 FragPos; // Receive the fragment position from the vertex shader

uniform vec3 lightPos; // The position of the light source
uniform vec3 viewPos; // The position of the camera

out vec4 FragColor;

void main()
{
    // Ambient lighting
    float ambientStrength = 1.0;
    vec3 ambient = ambientStrength * ourColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * ourColor;

    // Specular lighting
    float specularStrength = 5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
