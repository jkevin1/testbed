#version 330 core

layout(location = 0) in vec2 position;

out VSOutput
{
    vec3 posWS; // Position in world space
    vec4 posSS; // Position in screen space
    vec4 uv;    // Texture coordinates for each layer
} FSinput;

layout(std140) uniform WorldData {
    mat4 mvp;
    vec3 eye;
    float time;
    vec2 dimensions;
};

const vec2 scroll1 = 0.25 * vec2(cos(45), sin(45));
const vec2 scroll2 = 0.25 * vec2(cos(135), sin(135));
const float tileRate = 0.1;

void main()
{
    FSinput.posWS = vec3(position.x, 0, position.y);
    FSinput.posSS = mvp * vec4(FSinput.posWS, 1.0);
    FSinput.uv.xy = (position.xy + vec2(0.25, 0) * time) * tileRate;
    FSinput.uv.zw = (position.yx + vec2(0.25, 0) * time) * tileRate;
    gl_Position = FSinput.posSS;
}
