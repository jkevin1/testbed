#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out VSOutput
{
    vec3 position;
    vec3 normal;
    vec3 color;
} FSinput;

layout(std140) uniform WorldData {
    mat4 mvp;
    vec3 eye;
    float time;
    vec2 dimensions;
};

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    FSinput.position = position;
    FSinput.normal = vec3(0.0f, 0.0f, -1.0f);
    FSinput.color = color;
}
