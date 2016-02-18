#version 330 core

in VSOutput
{
    vec3 posWS; // Position in world space
    vec4 posSS; // Position in screen space
    vec4 uv;    // Texture coordinates for each layer
} FSinput;

out vec3 color;

uniform sampler2D gradient;
uniform sampler2D covariance;
uniform sampler2D backBuffer;
uniform sampler2D depth;
uniform vec3 light;
uniform bool rough = true;

layout(std140) uniform WorldData {
    mat4 mvp;
    vec3 eye;
    float time;
    vec2 dimensions;
};

const vec3 waterColor = vec3(57, 88, 121) / 255.0;
const float Ka = 0.2;
const float Kd = 0.4;
const float Ks = 0.4;
const float shininess = 64.0;
const float near = 0.1;
const float far = 100.0;
const float density = 1.0f;
const float stride = 1.0f;


float linearZ(float z) {
  float zn = 2.0 * z - 1.0;
  return (2 * near * far) / (far + near - z * (far - near));
}

// Samples and combines the two LEAN maps
void sampleLEAN(vec2 uv1, vec2 uv2, out vec2 G, out vec3 C) {
  vec2 G0 = texture(gradient, uv1).xy;
  vec2 G1 = texture(gradient, uv2).xy;
  vec3 C0 = texture(covariance, uv1).xyz;
  vec3 C1 = texture(covariance, uv2).xyz;
  C0 -= vec3(G0*G0, G0.x*G0.y);
  C1 -= vec3(G1*G1, G1.x*G1.y);
  G = (G0 + G1) / 2;
  C.xy = 0.25 * (C0.xy + C1.xy + 2.0*G0.xy*G1.xy);
  C.z = 0.25 * (C0.z + C1.z + G0.x*G1.y + G1.x*G0.y);
}



// Trace in screen space, if no intersection occurs, return background
vec3 traceSS(vec3 P, vec3 dP, vec3 background, vec2 covar) {
//  if (dP.y <= 0) return background;

  // Find starting position in screen space
  vec4 pixel = mvp * vec4(P, 1);
  vec3 pos = pixel.xyz / pixel.w;
  pos += 1;
  pos /= 2;

  // Project the ray onto the surface that is the furthest possible reflection
  if (dP.y < 0) dP *= 2.0 / -dP.y;
  if (dP.y > 0) dP *= 4.0 / dP.y;
//  if (dP.x > 20 || dP.x < -20) dP *= 20.0f / abs(dP.x); // clamp on x axis
//  if (dP.z > 20 || dP.z < -20) dP *= 20.0f / abs(dP.z); // clamp on z axis

  // Find another position along ray in screen space
  pixel = mvp * vec4(P + dP, 1);
  vec3 p2 = pixel.xyz / pixel.w;
  if (pixel.w < 0) return background;
  p2 += 1;
  p2 /= 2;

  // Calculate direction along ray
  vec3 dir = p2 - pos;
  dir.xy *= dimensions;
  float iterations = max(abs(dir.x), abs(dir.y));
  dir /= iterations;
  dir.xy /= dimensions;
  dir *= stride;
  iterations /= stride;

  float blur = 128 * sqrt(covar.x + covar.y);
  
  vec2 dPdx = blur * dir.xy;
  vec2 dPdy = blur * vec2(dir.y, -dir.x);
  float step = 1;
 
  float diff = 0;
  if (iterations > dimensions.y) iterations = dimensions.y;
  float intersect = 0.0f;
  for (float i = 0; i < iterations; i += step) {
    pos += dir * step;
    float depth = texture(depth, pos.xy).x;
    if (pos.x >= 1 || pos.x < 0 || pos.y >= 1 || pos.y < 0) break;
    diff = abs(depth - pos.z);
    if (depth < pos.z) {
      intersect = 1.0f;
      break;
    }
    step *= 1.05;
  }
  if (diff > 0.005) intersect = 0.0f;
  vec3 back;
  if (rough)
    back = textureGrad(backBuffer, pos.xy, dPdx*step, dPdy*step).xyz;
  else
    back = texture(backBuffer, pos.xy).xyz;
//  vec3 back = textureLod(backBuffer, pos.xy, 10).xyz;
  
  float edge = 1.0 - max(max(pos.x, 1.0-pos.x), max(pos.y, 1.0-pos.y));
  edge = clamp(edge * 16, 0, 1);

  return mix(background, back, intersect * log2(edge+1));
}

void main()
{
    vec3 pixel = FSinput.posSS.xyz / FSinput.posSS.w;
    pixel += 1;
    pixel /= 2;
    float depth = linearZ(texture(depth, pixel.xy).x);
    depth -= linearZ(pixel.z);
    float fog = exp(-depth * density);
    vec3 refracted = texture(backBuffer, pixel.xy).rgb;
    
    vec2 G; vec3 C;
    sampleLEAN(FSinput.uv.xy, FSinput.uv.zw, G, C);

    vec3 N = normalize(vec3(G.x, 1, G.y));
    vec3 L = normalize(light - FSinput.posWS);
    vec3 V = normalize(eye - FSinput.posWS);
    vec3 H = normalize(L + V);

    float diff = Kd * max(dot(L, N), 0.0);
   
    vec2 h = H.xz / H.y - G;
    float det = C.x*C.y - C.z*C.z;
    float e = (h.x*h.x*C.y + h.y*h.y*C.x - 2*h.x*h.y*C.z);
    float spec = (det <= 0) ?  0.0 : exp(-0.5*e/det) / sqrt(det);
   
    float fresnel = 0.05 + (1-0.05)*pow(1-dot(V, N), 5);
    
    refracted = traceSS(FSinput.posWS, refract(-V, N, 1/1.33f), refracted, vec2(0));
    refracted = mix((Ka + diff) * waterColor, refracted, fog);
    vec3 reflected = traceSS(FSinput.posWS, reflect(-V, N), vec3(spec) + vec3(0, 191.0f/255.0f, 1), C.xy);
    color = mix(refracted, reflected, fresnel);
//    color = reflected;
//    color = refracted;
}
