#version 410 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform int uLightCount;
uniform vec3 uAmbientColor;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uLightPositions[8];
uniform vec3 uLightColors[8];
uniform float uLightIntensities[8];

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vWorldNormal);
    vec3 baseColor = texture(uTexture, vTexCoord).rgb;
    vec3 lighting = uAmbientColor;
    float sunDiffuse = max(dot(normal, normalize(-uSunDirection)), 0.0);
    lighting += uSunColor * sunDiffuse * uSunIntensity;

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3 toLight = uLightPositions[i] - vWorldPosition;
        float distanceToLight = length(toLight);
        float diffuse = 0.0;
        if (distanceToLight > 0.0001)
        {
            diffuse = max(dot(normal, toLight / distanceToLight), 0.0);
        }
        float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);
        lighting += uLightColors[i] * diffuse * uLightIntensities[i] * attenuation;
    }

    FragColor = vec4(baseColor * lighting, 1.0);
}
