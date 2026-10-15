#version 410 core

layout (location = 0) in vec3 aPosition;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vDirection;

void main()
{
    vDirection = aPosition;
    vec4 clipPosition = uProjection * uView * vec4(aPosition, 1.0);
    gl_Position = clipPosition.xyww;
}
