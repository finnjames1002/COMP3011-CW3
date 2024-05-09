#version 330 core

in vec2 TexCoord; // Add this line
in vec3 Normal; // Receive the normal from the vertex shader
in vec3 FragPos; // Receive the fragment position from the vertex shader
in vec4 FragPosProjectedLightSpace;

uniform sampler2D shadowMap;
uniform sampler2D Texture;
uniform vec3 lightPos; // The position of the light source
uniform vec3 viewPos; // The position of the camera

out vec4 fragColour;

float shadowOnFragment(vec4 FragPosProjectedLightSpace, vec3 lightDir)
    {
    vec3 ndc = FragPosProjectedLightSpace.xyz / FragPosProjectedLightSpace.w;
    vec3 ss = (ndc + 1.0) * 0.3;

    float fragDepth = ss.z;

    float shadow = 0.f;
    float litdepth = texture(shadowMap, ss.xy).r;
    float bias = max(0.15 * (1.0 - dot(Normal, lightDir)), 0.015);
    shadow = fragDepth - bias > litdepth ? 0.5 : 0.0;
    return shadow;
}

void main()
{
    // Ambient lighting
    float ambientStrength = 0.8f;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Specular lighting
    float specularStrength = 1.f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    vec4 texColor = texture(Texture, TexCoord);
    float shadow = shadowOnFragment(FragPosProjectedLightSpace, lightDir);
    vec3 result = (ambient + ((1.f - shadow) * (diffuse + specular))) * texColor.rgb; // Apply the shadow
    fragColour = vec4(result, texColor.a); // Use alpha value from texture
}

    
