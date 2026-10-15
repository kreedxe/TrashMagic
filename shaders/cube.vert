#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uTextureScale;

out vec3 vWorldNormal;
out vec3 vWorldPosition;
out vec2 vTexCoord;
out vec4 vLightSpacePosition;

uniform mat4 uLightSpace;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vLightSpacePosition = uLightSpace * worldPosition;
    vec2 textureScale;
    if (abs(aNormal.y) > 0.5)
    {
        textureScale = uTextureScale.xz;
    }
    else if (abs(aNormal.x) > 0.5)
    {
        textureScale = uTextureScale.zy;
    }
    else
    {
        textureScale = uTextureScale.xy;
    }
    vTexCoord = aTexCoord * textureScale;
    gl_Position = uProjection * uView * worldPosition;
}
