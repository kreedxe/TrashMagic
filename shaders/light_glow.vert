#version 410 core

layout (location = 0) in vec2 aPosition;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPosition;
uniform float uRadius;

out vec2 vPosition;

void main()
{
    vPosition = aPosition;
    vec4 viewPosition = uView * vec4(uPosition, 1.0);
    viewPosition.xy += aPosition * uRadius;
    gl_Position = uProjection * viewPosition;
}
