#include "Texture.hpp"
#include "LEAN.hpp"
#include "lodepng.h"
#include "stdio.h"

GLuint Graphics::createTexture(const char* filename)
{
    std::vector<unsigned char> data;
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, filename);
    if (error) {
        fprintf(stderr, "[Texture] Error loading %s\n", filename);
        return 0;
    }
    
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    printf("Created texture %u (%ux%u)\n", id, width, height);
    return id;
};

GLuint Graphics::loadLEANGradient(const LEANMap& lean)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lean.width, lean.height, 0, GL_RGB, GL_FLOAT, lean.grad);
    glGenerateMipmap(GL_TEXTURE_2D);
    printf("Created gradient map %u (%ux%u)\n", id, lean.width, lean.height);
    return id;
}

GLuint Graphics::loadLEANCovariance(const LEANMap& lean)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lean.width, lean.height, 0, GL_RGB, GL_FLOAT, lean.covar);
    glGenerateMipmap(GL_TEXTURE_2D);
    printf("Created covariance map %u (%ux%u)\n", id, lean.width, lean.height);
    return id;
}
