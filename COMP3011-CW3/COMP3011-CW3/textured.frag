#version 330 core

in vec2 TexCoord; // Add this line
in vec3 Normal; // Receive the normal from the vertex shader
in vec3 FragPos; // Receive the fragment position from the vertex shader
in vec4 FragPosProjectedLightSpace;

uniform sampler2D shadowMap;
uniform sampler2D Texture;
uniform sampler2D texture_specular;
uniform vec3 lightPos; // The position of the light source
uniform vec3 viewPos; // The position of the camera
uniform vec3 lightDir; // The direction of the light source
uniform float lightIntensity; // Light intensity dependent on Y position

out vec4 fragColour;

float shadowCalculation(vec4 fragPosLightSpace)
{
    vec3 ndc = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 ss = (ndc+1) * 0.5;

    float fragDepth = ss.z;

    float shadow = 0.f;
    float litDepth = texture(shadowMap, ss.xy).r;
    float bias = 0.005;
    shadow = fragDepth - bias > litDepth ? 1.0 : 0.0;
    if (fragDepth > 1.0)
        shadow = 0.0;
   
    return shadow;
}

void main()
{
    // Ambient lighting
    float ambientStrength = 0.8f;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Specular lighting
    float specularStrength = 1.f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = specularStrength * spec * texture(texture_specular, TexCoord).rgb; // Use the specular map

      vec4 texColor = texture(Texture, TexCoord);
    float shadow = shadowCalculation(FragPosProjectedLightSpace);
    vec3 result = (ambient + ((1.f - shadow) * (diffuse + specular))) * texColor.rgb * lightIntensity; // Multiply by light intensity
    fragColour = vec4(result, texColor.a); // Use alpha value from texture

}
