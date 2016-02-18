#version 330 core

layout(location = 0) in vec2 position;

uniform sampler2D heightmap;

out VSOutput
{
    vec3 position;
    vec3 normal;
    vec3 color;
} FSinput;

layout (std140) uniform WorldData {
    mat4 mvp;
    vec3 eye;
    float time;
    vec2 dimensions;
};

const vec2 size = vec2(2.0, 0.0);
const vec3 offset = vec3(-1.0, 0.0, 1.0) / 1023.0;
const float wsOff = 40.0 / 1023.0;

void main()
{
    vec2 uv = position / 40;
    uv.x += 0.5;
    uv.y += 0.5;
    
    float y = texture(heightmap, uv).r;
    FSinput.position = vec3(position.x, (y-.25)*4, position.y);
    gl_Position = mvp * vec4(FSinput.position, 1.0);

    float x1 = texture(heightmap, uv + offset.xy).r * 4;
    float x2 = texture(heightmap, uv + offset.zy).r * 4;
    float y1 = texture(heightmap, uv + offset.yx).r * 4;
    float y2 = texture(heightmap, uv + offset.yz).r * 4;
    vec3 dx = vec3(position.x+wsOff, x2, position.y) - vec3(position.x-wsOff, x1, position.y);
    vec3 dy = vec3(position.x, y2, position.y+wsOff) - vec3(position.x, y1, position.y-wsOff);
    dx = normalize(dx);
    dy = normalize(dy);
    FSinput.normal = cross(dx, dy);
    FSinput.normal.y *= -1;
    FSinput.color = mix(vec3(237, 201, 175), vec3(44, 176, 55), y) / 256.0;
}
