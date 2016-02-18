#include "LEAN.hpp"
#include <stdio.h>
#include "lodepng.h"

// A lean file consists of the raw dimensions followed by raw gradient
// and raw covariance buffers

using glm::vec3;

LEANMap::LEANMap(const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Error loading %s\n", filename);
    width = 0;
    height = 0;
    grad = nullptr;
    covar = nullptr;
    return;
  }
  fread(&width, sizeof(unsigned), 1, file);
  fread(&height, sizeof(unsigned), 1, file);
  grad = new vec3[width * height];
  covar = new vec3[width * height];
  fread(grad, sizeof(vec3), width * height, file);
  fread(covar, sizeof(vec3), width * height, file);
  fclose(file);
}

LEANMap::LEANMap(unsigned width, unsigned height) {
  this->width = width;
  this->height = height;
  grad = new vec3[width * height];
  covar = new vec3[width * height];
}

LEANMap::LEANMap(LEANMap&& ref) {
  width = ref.width;
  height = ref.height;
  grad = ref.grad;
  covar = ref.covar;
  ref.width = 0;
  ref.height = 0;
  ref.grad = nullptr;
  ref.covar = nullptr;
}

LEANMap::~LEANMap() {
  delete[] grad;
  delete[] covar;
}

LEANMap LEANMap::generate(const char* filename, float scale) {
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, filename);

  if (error) {
    fprintf(stderr, "Error loading %s: %s\n", filename, lodepng_error_text(error));
    return LEANMap(0, 0);
  }
  
  // Lambda function to sample a wrapped texturee
  auto sample = [&](int x, int y) {
    if (x < 0) x += width;
    if (x >= (int)width) x -= width;
    if (y < 0) y += height;
    if (y >= (int)height) y -= height;
    unsigned char v = image[4*y*width + 4*x];
    return scale * (v / 255.0f);
  };

  LEANMap output(width, height);
  for (int x = 0; x < (int)width; x++) {
    for (int y = 0; y < (int)height; y++) {
      // Sample texture
      float x1 = sample(x-1, y);
      float x2 = sample(x+1, y);
      float y1 = sample(x, y-1);
      float y2 = sample(x, y+1);
      float center = sample(x, y);
      
      // Calculate normal
      vec3 dx = vec3(x+1, y, x2) - vec3(x-1, y, x1);
      vec3 dy = vec3(x, y+1, y2) - vec3(x, y-1, y1);
      dx = glm::normalize(dx);
      dy = glm::normalize(dy);
      vec3 normal = glm::cross(dx, dy);
      normal /= normal.z;

      float invS = 1.0f / 1024.0f;
      // Store the results in memory
      unsigned index = y*width + x;
      output.grad[index] = vec3(normal.x, normal.y, center);
      output.covar[index] = vec3(normal.x*normal.x + invS, normal.y*normal.y + invS, normal.x*normal.y);
    }
  }

  return output;
}

void LEANMap::write(const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    fprintf(stderr, "Failed to write %s\n", filename);
    return;
  }
  fwrite(&width, sizeof(unsigned), 1, file);
  fwrite(&height, sizeof(unsigned), 1, file);
  fwrite(grad, sizeof(glm::vec3), width * height, file);
  fwrite(covar, sizeof(glm::vec3), width * height, file);
  fclose(file);
}
