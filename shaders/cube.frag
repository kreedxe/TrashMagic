#version 410 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vTexCoord;
in vec4 vLightSpacePosition;

uniform sampler2D uTexture;
uniform sampler2D uShadowMap;
uniform int uLightCount;
uniform bool uShadowsEnabled;
uniform vec3 uAmbientColor;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uViewPosition;
uniform float uRoughness;
uniform float uSpecularStrength;
uniform vec3 uLightPositions[8];
uniform vec3 uLightColors[8];
uniform float uLightIntensities[8];
uniform float uLightRadii[8];

out vec4 FragColor;

float calculateShadow(vec3 normal, vec3 lightDirection)
{
    if (!uShadowsEnabled)
    {
        return 0.0;
    }
    vec3 projected = vLightSpacePosition.xyz / vLightSpacePosition.w;
    projected = projected * 0.5 + 0.5;
    if (projected.z > 1.0
        || projected.x < 0.0 || projected.x > 1.0
        || projected.y < 0.0 || projected.y > 1.0)
    {
        return 0.0;
    }

    float bias = max(0.0012 * (1.0 - dot(normal, lightDirection)), 0.00025);
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(
                uShadowMap,
                projected.xy + vec2(x, y) * texelSize).r;
            shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main()
{
    vec3 normal = normalize(vWorldNormal);
    vec3 baseColor = texture(uTexture, vTexCoord).rgb;
    vec3 viewDirection = normalize(uViewPosition - vWorldPosition);
    vec3 ambient = uAmbientColor * mix(0.7, 1.15, normal.y * 0.5 + 0.5);
    vec3 directLighting = vec3(0.0);
    vec3 sunDirection = normalize(-uSunDirection);
    float roughness = clamp(uRoughness, 0.04, 1.0);
    float shininess = mix(128.0, 4.0, roughness * roughness);
    float sunDiffuse = max(dot(normal, sunDirection), 0.0);
    vec3 sunHalfway = normalize(sunDirection + viewDirection);
    float sunSpecular = pow(max(dot(normal, sunHalfway), 0.0), shininess);
    float shadow = calculateShadow(normal, sunDirection);
    directLighting += uSunColor * uSunIntensity * (1.0 - shadow)
        * (baseColor * sunDiffuse + vec3(uSpecularStrength * sunSpecular));

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3 toLight = uLightPositions[i] - vWorldPosition;
        float distanceToLight = length(toLight);
        if (distanceToLight > 0.0001)
        {
            vec3 lightDirection = toLight / distanceToLight;
            vec3 halfwayDirection = normalize(lightDirection + viewDirection);
            float diffuse = max(dot(normal, lightDirection), 0.0);
            float specular = pow(
                max(dot(normal, halfwayDirection), 0.0),
                shininess);
            float normalizedDistance = distanceToLight / uLightRadii[i];
            float rangeFade = clamp(1.0 - pow(normalizedDistance, 4.0), 0.0, 1.0);
            float attenuation = rangeFade * rangeFade
                / (1.0 + distanceToLight * distanceToLight);
            directLighting += uLightColors[i] * uLightIntensities[i] * attenuation
                * (baseColor * diffuse + vec3(uSpecularStrength * specular));
        }
    }

    FragColor = vec4(baseColor * ambient + directLighting, 1.0);
}
