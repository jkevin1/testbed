#version 330 core

in VSOutput
{
    vec3 position;
    vec3 normal;
    vec3 color;
} FSinput;

out vec3 color;

uniform vec3 light;

layout (std140) uniform WorldData {
    mat4 mvp;
    vec3 eye;
    float time;
    vec2 dimensions;
};

const float Ka = 0.5;
const float Kd = 0.2;
const float Ks = 0.0;
const float shininess = 16.0;

void main()
{
    vec3 L = normalize(light - FSinput.position);
    vec3 V = normalize(eye - FSinput.position);
    vec3 H = normalize(L + V);
    float diff = Kd * max(dot(L, FSinput.normal), 0.0);
    float spec = Ks * pow(max(dot(H, FSinput.normal), 0.0), shininess);
    color = (Ka + diff) * FSinput.color + vec3(spec);
}
