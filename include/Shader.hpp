#ifndef Shader_HPP
#define Shader_HPP

#include <Graphics.hpp>
#include "UniformBlock.hpp"
#include <list>

namespace Graphics
{

class Variable {
public:
    template <typename T>   // TODO implement for all glUniform* varieties, bindless textures?, etc
    void set(const T& val);
private:
    friend class Shader;
    GLuint location;
    GLuint program;
};

class Shader {
public:
    Shader() : id(0) { }
    Shader(Shader&& shader);
    Shader& operator=(Shader&& shader);
    ~Shader();
    void use() const;
    void release();
    Variable operator[](const char* name);
    void setBinding(const char* name, GLuint binding);
private:
    friend Shader createProgram(const std::list<GLuint>& shaders);
    Shader(GLuint id);
    GLuint id;
};

GLuint loadShader(const char* filename, GLenum type);
GLuint createShader(const char* source, GLenum type);

Shader createProgram(const std::list<GLuint>& shaders);

}

#endif
