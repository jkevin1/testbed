#ifndef Texture_HPP
#define Texture_HPP

#include "Graphics.hpp"

class LEANMap;
namespace Graphics
{
    GLuint createTexture(const char* filename);
    GLuint createTexture(int width, int height);

    GLuint loadLEANGradient(const LEANMap& lean);
    GLuint loadLEANCovariance(const LEANMap& lean);
}

#endif
