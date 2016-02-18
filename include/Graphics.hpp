#ifndef Graphics_HPP
#define Graphics_HPP

#include <GL/glew.h>

namespace Graphics
{

// Initialize OpenGL
void initialize(bool debug=false);

// Print the version, vendor, and renderer
void printContextData();

// Release resources
void shutdown();

}

#endif // Graphics_HPP
