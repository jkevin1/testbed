#include "Models.hpp"
#include "Shader.hpp"
#include "Graphics.hpp"
#include <glm/glm.hpp>

enum Model { SCENERY=0, TERRAIN=1, OCEAN=2, NUM, UNIFORM=6 };

GLuint shaders[NUM];
GLuint vertexArrays[NUM];
GLuint buffers[NUM*2+1];
GLsizei numIndices[NUM];

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct Transform
{
    glm::mat4 vpMatrix;
};

struct Lighting
{
    glm::vec3 eye;
    glm::vec3 light;
};

void buildScenery();
void buildTerrain();
void buildOcean();

void Models::initialize()
{
    glGenVertexArrays(3, vertexArrays);
    glGenBuffers(NUM*2+1, buffers);

    buildScenery();
    buildTerrain();
    buildOcean();

    // Initialize scenery
    glBindVertexArray(vertexArrays[SCENERY]);

    glBindBuffer(GL_ARRAY_BUFFER, vertexArrays[SCENERY]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrays[SCENERY] * 2);
    glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));

     // Initialize terrain
    glBindVertexArray(vertexArrays[TERRAIN]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrays[TERRAIN]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrays[TERRAIN] * 2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

     // Initialize ocean
    glBindVertexArray(vertexArrays[OCEAN]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrays[OCEAN]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrays[OCEAN] * 2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    
    // Clear bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load shaders
    GLuint sceneryVS = Graphics::loadShader("res/scenery.vert", GL_VERTEX_SHADER);
    GLuint terrainVS = Graphics::loadShader("res/terrain.vert", GL_VERTEX_SHADER);
    GLuint oceanVS = Graphics::loadShader("res/ocean.vert", GL_VERTEX_SHADER);
    GLuint phongFS = Graphics::loadShader("res/phong.frag", GL_FRAGMENT_SHADER);
    GLuint oceanFS = Graphics::loadShader("res/ocean.frag", GL_FRAGMENT_SHADER);
    
    // Create programs
    shaders[SCENERY] = Graphics::createProgram({sceneryVS, phongFS});
    shaders[TERRAIN] = Graphics::createProgram({terrainVS, phongFS});
    shaders[OCEAN] = Graphics::createProgram({oceanVS, oceanFS});

    // Release shaders
    glDeleteShader(sceneryVS);
    glDeleteShader(terrainVS);
    glDeleteShader(oceanVS);
    glDeleteShader(phongFS);
    glDeleteShader(oceanVS);

    // Create uniform buffers
    glBindBuffer(GL_UNIFORM_BUFFER, buffers[UNIFORM]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform) + sizeof(Lighting), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    enum Bindings { TRANSFORM=2, LIGHTING=3 };
    for (int i = 0; i < NUM; i++)
    {
        glUseProgram(shaders[i]);
        GLuint index = glGetUniformBlockIndex(shaders[i], "Transform");
        glUniformBlockBinding(shaders[i], index, TRANSFORM);
        index = glGetUniformBlockIndex(shaders[i], "Lighting");
        glUniformBlockBinding(shaders[i], index, LIGHTING);
    }
    glUseProgram(0);
    glBindBufferRange(GL_UNIFORM_BUFFER, TRANSFORM, buffers[UNIFORM], 0, sizeof(Transform));
    glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTING, buffers[UNIFORM], sizeof(Transform), sizeof(Lighting)); 
}

void Models::update(const glm::mat4& vpMatrix, const glm::vec3& eyePos)
{
    glBindBuffer(GL_UNIFORM_BUFFER, buffers[UNIFORM]);
    GLubyte* data = (GLubyte*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Transform) + sizeof(Lighting), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    Transform& t = *((Transform*)(data+0));
    Lighting& l = *((Lighting*)(data + sizeof(Transform)));
    t.vpMatrix = vpMatrix;
    l.eye = eyePos;
    l.light = glm::vec3(0);
    glUnmapBuffer(GL_UNIFORM_BUFFER); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);   
}

void Models::draw()
{
    // Render scenery
    glUseProgram(shaders[SCENERY]);
    glBindVertexArray(vertexArrays[SCENERY]);
//    glDrawElements(GL_TRIANGLES, numIndices[SCENERY], GL_UNSIGNED_INT, 0);
    // Will be instanced ^^

    // Render terrain
    glUseProgram(shaders[TERRAIN]);
    glBindVertexArray(vertexArrays[TERRAIN]);
    glDrawElements(GL_TRIANGLES, numIndices[TERRAIN], GL_UNSIGNED_INT, 0);

    // Render ocean
    glUseProgram(shaders[OCEAN]);
    glBindVertexArray(vertexArrays[OCEAN]);
    glDrawElements(GL_TRIANGLES, numIndices[OCEAN], GL_UNSIGNED_INT, 0);

    // Clear bindings
    glBindVertexArray(0);
    glUseProgram(0);
}

void Models::release()
{
    glDeleteProgram(shaders[SCENERY]);
    glDeleteProgram(shaders[TERRAIN]);
    glDeleteProgram(shaders[OCEAN]);
    glDeleteBuffers(NUM*2, buffers);
    glDeleteVertexArrays(NUM, vertexArrays);
}

void buildScenery() {}

void buildTerrain()
{
    glm::vec2 vertices[41*41]; int arrayIndex = 0;
    GLuint indices[40*40*2*3];
   
    for (float y = -20.0f; y <= 20.0f; y += 1.0f)
        for (float x = -20.0f; x <= 20.0f; x += 1.0f)
            vertices[arrayIndex++] = glm::vec2(x, y);

    arrayIndex = 0;
    for (unsigned i = 0; i < 40; i++)
    {
        for (unsigned j = 0; j < 40; j++)
        {
            const unsigned width = 41;
            GLuint index = i*width + j;
            indices[arrayIndex++] = index;
            indices[arrayIndex++] = index + width;
            indices[arrayIndex++] = index + 1;
            
            indices[arrayIndex++] = index + 1;
            indices[arrayIndex++] = index + width;
            indices[arrayIndex++] = index + width + 1;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers[SCENERY]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[SCENERY*2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    numIndices[TERRAIN] = sizeof(indices) / sizeof(GLuint);
}

void buildOcean()
{
    glm::vec2 vertices[] = 
    { glm::vec2(-20.0f, -20.0f), glm::vec2(-20.0f, 20.0f), glm::vec2(20.0f, -20.0f), glm::vec2(20.0f, 20.0f) };
    GLuint indices[] = { 0, 1, 2, 2, 1, 3 };
    
    glBindBuffer(GL_ARRAY_BUFFER, buffers[SCENERY]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[SCENERY*2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    numIndices[TERRAIN] = sizeof(indices) / sizeof(GLuint);
}
