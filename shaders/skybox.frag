#version 410 core

in vec3 vDirection;

uniform samplerCube uCubemap;
uniform float uBrightness;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(uCubemap, normalize(vDirection)).rgb * uBrightness, 1.0);
}
