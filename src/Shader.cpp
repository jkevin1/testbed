#include "Shader.hpp"
#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>

GLuint Graphics::loadShader(const char* filename, GLenum type)
{
    printf("Loading shader: %s\n", filename);
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        perror(filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<GLchar> source(size + 1);
    fread(source.data(), 1, size, file);
    source[size] = '\0';
    fclose(file);

    return createShader(source.data(), type);
}

GLuint Graphics::createShader(const GLchar* source, GLenum type)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {// Message handled by debug layer
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> message(length + 1);
        glGetShaderInfoLog(shader, length, nullptr, message.data());
        fprintf(stderr, "%s\n", message.data());
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

Graphics::Shader Graphics::createProgram(const std::list<GLuint>& shaders)
{
    GLuint program = glCreateProgram();
    
    for (auto& shader: shaders)
        glAttachShader(program, shader);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {// Message handled by debug layer
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> message(length + 1);
        glGetProgramInfoLog(program, length, nullptr, message.data());
        fprintf(stderr, "%s\n", message.data());
        glDeleteProgram(program);
        return std::move(Shader());
    }
    printf("Created program %u\n", program);
    return std::move(Shader(program));
}

Graphics::Shader::Shader(Shader&& ref) : id(ref.id)
{
    ref.id = 0;
}

Graphics::Shader& Graphics::Shader::operator=(Shader&& ref)
{
    id = ref.id;
    ref.id = 0;
    return *this;
}

Graphics::Shader::Shader(GLuint id) : id(id)
{
    // Construct from existing id
}

Graphics::Shader::~Shader()
{
    // TODO invalidate variables
    release();
}

void Graphics::Shader::use() const
{
    glUseProgram(id);
}

void Graphics::Shader::release()
{
    if (id)
    {
        printf("Releasing shader %u\n", id);
        glDeleteProgram(id);
        id = 0;
    }
}

Graphics::Variable Graphics::Shader::operator[](const char* name)
{
    Variable var;
    var.program = id;
    var.location = glGetUniformLocation(id, name);
    return var;
}

void Graphics::Shader::setBinding(const char* name, GLuint binding) {
    GLuint index = glGetUniformBlockIndex(id, name);
    glUniformBlockBinding(id, index, binding);
    printf("Bound %s (%u) to %u\n", name, index, binding);
}

namespace Graphics {

template <>
void Variable::set(const float& value)
{
    glProgramUniform1f(program, location, value);
}

template <>
void Variable::set(const int& value)
{
    glProgramUniform1i(program, location, value);
}

template <>
void Variable::set(const glm::vec3& value)
{
    glProgramUniform3fv(program, location, 1, &value[0]);
}

template <>
void Variable::set(const glm::mat4& value)
{
    glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, &value[0][0]);
}

template <>
void Variable::set(const glm::vec2& value)
{
    glProgramUniform2fv(program, location, 1, &value[0]);
}

template <>
void Variable::set(const bool& value)
{
    glProgramUniform1i(program, location, value);
}

}

