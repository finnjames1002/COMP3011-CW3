#version 330 core

in vec2 TexCoord; // Coordinates of texture
in vec3 Normal; // Normal position
in vec3 FragPos; // Fragment position
in vec4 FragPosProjectedLightSpace; // Fragment position in light space

uniform sampler2D shadowMap; // Shadow map
uniform sampler2D Texture; // Texture map
uniform sampler2D texture_specular; // Specular map
uniform sampler2D texture_emit; // Emit map

uniform vec3 lightPos; // The position of the sun
uniform vec3 viewPos; // The position of the camera
uniform vec3 lightDir; // The direction of the sun
uniform float lightIntensity; // Light intensity of sun
uniform float emitIntensity; // Intensity of the street light
uniform vec3 pointLightPos; // Position of the street light
uniform vec3 pointLightColor; // Color of the street light

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
    if (fragDepth > 1.0) // If the fragment is outside the light view
        shadow = 0.0;
   
    return shadow;
}

void main()
{
    vec3 lightDir2 = pointLightPos - FragPos; // Direction from fragment to street light
    float distance = length(lightDir2);
    lightDir2 = normalize(lightDir2);
    float attenuation = 1.0 / (1.0 + 0.10 * distance + 0.03 * distance * distance); // Physics stuff for light to fall off with distance

    // Ambient lighting
    float ambientStrength = 0.8f;
    vec3 ambient = vec3(0.0, 0.0, 0.0); // Default ambient light color
    
    if (distance < 1.0) { // If the fragment is close to the street light
        ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
    }
    else if (lightIntensity < 0.2) { // If the fragment is far from the street light and it is dark
        ambient += pointLightColor * attenuation;
    }
    else { // If the fragment is far from the street light and it is bright
        ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
    }
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); // Default diffuse light color * diffuse factor

    // Specular lighting
    float specularStrength = 1.f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = specularStrength * spec * texture(texture_specular, TexCoord).rgb; // Default specular light color * specular factor. I honestly don't know if this made any difference

    vec4 texColor = texture(Texture, TexCoord); // Get texture color
    float shadow = shadowCalculation(FragPosProjectedLightSpace); // Calculate shadow
    vec4 emitColor = texture(texture_emit, TexCoord); // Get emit color
    vec3 result = (ambient * texColor.rgb + ((1.f - shadow) * (diffuse + specular)) * lightIntensity + emitColor.rgb * emitIntensity) * texColor.rgb; // Calculate final color
    if (lightIntensity < 0.2) { // Overwrite the final color if the street light is on
        if (distance > 1.0) {
            result = (ambient * texColor.rgb + ((1.f - shadow) * (diffuse + specular)) + emitColor.rgb * emitIntensity) * texColor.rgb; 
        }
    }
    fragColour = vec4(result, texColor.a); // Set final color including alpha map
}

