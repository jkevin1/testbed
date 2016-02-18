#ifndef LEAN_HPP
#define LEAN_HPP

#include "Texture.hpp"
#include <glm/glm.hpp>

class LEANMap {
 public:
  // Load LEAN map from files
  LEANMap(const char* filename);
  // Allocate new LEAN map
  LEANMap(unsigned width, unsigned height);
  // Move constructor
  LEANMap(LEANMap&& ref);
  // Destructor
  virtual ~LEANMap();

  // Create a leanmap from a bump map file
  static LEANMap generate(const char* filename, float scale);
  // Write a lean map to file
  void write(const char* filename);

  // Sampling functions
  glm::vec3 gradient(unsigned x, unsigned y) { return grad[y*width + x]; }
  glm::vec3 covariance(unsigned x, unsigned y) { return covar[y*width + x]; }
 private:
  friend GLuint Graphics::loadLEANGradient(const LEANMap&);
  friend GLuint Graphics::loadLEANCovariance(const LEANMap&);
  unsigned width, height;
  glm::vec3* grad;  // gx, gy, height
  glm::vec3* covar; // xx, xy, yy
};

#endif // LEAN_HPP
