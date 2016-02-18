#ifndef Mesh_HPP
#define Mesh_HPP

#include "Graphics.hpp"
#include "Buffer.hpp"
#include <vector>

struct Attrib
{
    GLuint size;
    GLenum type;
};

template<class Vertex>
struct Mesh
{
    std::vector<Attrib> attributes;
    std::vector<Vertex> vertices;
};

template<class Vertex>
struct IndexedMesh
{
    std::vector<Attrib> attributes;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

namespace Graphics
{

    struct Surface
    {
        GLenum prim;// Primitive type (currently only GL_TRIANGLES)
        GLuint vao; // Vertex array object
        GLuint vbo; // Vertex buffer handle
        GLuint ibo; // Index buffer handle
        GLsizei num;// Number of elements or vertices
    };

    Surface createSurface(const Buffer& data, const std::vector<Attrib>& attributes);
    Surface createSurface(const Buffer& data, const std::vector<Attrib>& attributes, const std::vector<GLuint>& indices);

    template <class Vertex>
    Surface createSurface(const Mesh<Vertex>& mesh) 
    {
        return createSurface(Buffer(mesh.vertices), mesh.attributes);
    }

    template <class Vertex>
    Surface createSurface(const IndexedMesh<Vertex>& mesh)
    {
        return createSurface(Buffer(mesh.vertices), mesh.attributes, mesh.indices);
    }

    void drawSurface(const Surface& surface);

    void deleteSurface(Surface& surface);
}

#endif // Mesh_HPP
