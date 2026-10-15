#version 410 core

in vec2 vPosition;

uniform vec3 uColor;
uniform float uIntensity;

out vec4 FragColor;

void main()
{
    float distanceFromCenter = length(vPosition);
    float edgeFade = 1.0 - smoothstep(0.25, 1.0, distanceFromCenter);
    float glow = exp(-distanceFromCenter * 4.5) * edgeFade;
    FragColor = vec4(uColor * uIntensity * glow, glow);
}
