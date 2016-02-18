#include "Mesh.hpp"


Graphics::Surface Graphics::createSurface(const Buffer& data, const std::vector<Attrib>& attributes)
{
    Surface surface = {GL_TRIANGLES, 0, 0, 0, GLsizei(data.num)};
    glGenBuffers(1, &surface.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, surface.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size, data.getPointerAs<GLubyte>(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &surface.vao);
    glBindVertexArray(surface.vao);
    glBindBuffer(GL_ARRAY_BUFFER, surface.vbo);

    GLubyte* offset = 0;
    for (size_t i = 0; i < attributes.size(); i++)
    {
        const Attrib& attrib = attributes[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attrib.size, attrib.type, GL_FALSE, data.stride, offset);
        offset += attrib.size * 4; // TODO support things other than 32 bit
    }

    return surface;
}

Graphics::Surface Graphics::createSurface(const Buffer& data, const std::vector<Attrib>& attributes, const std::vector<GLuint>& indices)
{
    Surface surface = createSurface(data, attributes);

    // Can copy from forwarded function, might be faster

    glGenBuffers(1, &surface.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
    surface.num = indices.size();

    return surface;
}

void Graphics::drawSurface(const Surface& surface)
{
    if (!surface.vao) return;
    glBindVertexArray(surface.vao);
    if (surface.ibo)
        glDrawElements(surface.prim, surface.num, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(surface.prim, 0, surface.num);
    // unbind?
}

void Graphics::deleteSurface(Graphics::Surface& surface)
{
    glDeleteBuffers(1, &surface.vbo);
    glDeleteBuffers(1, &surface.ibo);
    glDeleteVertexArrays(1, &surface.vao);
    surface.vbo = 0;
    surface.vao = 0;
    surface.ibo = 0;
    surface.num = 0;
}

// drawSurface bind vao, glDrawArrays(GL_TRIANGLES, 0, surface.num)
