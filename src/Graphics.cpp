#include "Graphics.hpp"
#include "Testbed.hpp"
#include <stdio.h>

namespace {
    void glError(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* msg, const void*)
    {
//        fprintf(stderr, "OpenGL Error:%s\n", msg); // TODO print formatting based on parameters
    }
}

void Graphics::initialize(bool debug)
{
    printf("[Graphics] Initializing\n");
    // TODO maybe set up some trace callbacks?
    
    // Initialize OpenGL bindings
    glewExperimental = GL_TRUE;
    Testbed::assert(glewInit() == GLEW_OK, "[Graphics] Failed to initialize GLEW");
    
    // Register debug callback with OpenGL
    if (debug)
    {
        printf("[Graphics] Enabling debug output\n");
        glEnable(GL_DEBUG_OUTPUT);
        // TODO limit some messages from being sent
        glDebugMessageCallback(&glError, nullptr);
    }
}

void Graphics::printContextData()
{
    printf("[Graphics] OpenGL version:  %s\n", glGetString(GL_VERSION));
    printf("[Graphics] OpenGL vendor:   %s\n", glGetString(GL_VENDOR));
    printf("[Graphics] OpenGL renderer: %s\n", glGetString(GL_RENDERER));
}

void Graphics::shutdown()
{
    printf("[Graphics] Shutting down\n");
}
