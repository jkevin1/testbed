#include "Testbed.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>
#undef assert

void initialize();
void reloadShaders();
void resize(int x, int y);
void update(double dt);
void render();
void shutdown();

// Vertices for test buffer

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

Mesh<Vertex> modelData = {
    {{3, GL_FLOAT}, {3, GL_FLOAT}},
    {{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f)},
     {glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f)},
     {glm::vec3( 0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f)}}
};

Mesh<glm::vec2> terrainData = {{{2, GL_FLOAT}}, {}};


Mesh<glm::vec2> oceanData = {
    {{2, GL_FLOAT}},
    {glm::vec2(-20.0f, -20.0f), glm::vec2(-20.0f, 20.0f), glm::vec2(20.0f, -20.0f),
     glm::vec2( 20.0f, -20.0f), glm::vec2(-20.0f, 20.0f), glm::vec2(20.0f, 20.0f)}
};

#define SENSITIVITY 0.01f
Camera camera(0,0,0,0);

// Framebuffer and color/depth targets TODO wrap in RenderTarget class or something
//GLuint fbo = 0;
//GLuint clr = 0;
//GLuint dpt = 0;

// Renderable surfaces  TODO other forms of surfaces (Instanced, Indexing, etc)
Graphics::Surface model{0};
Graphics::Surface terrain{0};
Graphics::Surface ocean{0};

// Shaders for different materials TODO pack into material object, use pipeline objects
GLuint shaderModel = 0;
GLuint shaderTerrain = 0;
GLuint shaderOcean = 0;

// Uniforms TODO pack shared data into UBO
GLuint mvpModel = 0;
GLuint mvpTerrain = 0;
GLuint mvpOcean = 0;

// Textures TODO use samplers
GLuint heightmap;

// Global transformation matrix
glm::mat4 mvp;

int main(int argc, char* argv[])
{
    initialize();

    double prevTime, currTime = Testbed::getTime(); // TODO wrap in timer or fpscounter class
    while (Testbed::running())
    {
        prevTime = currTime;
        currTime = Testbed::getTime(); 
        update(currTime - prevTime);
        render();
    }

    shutdown();
}

void initialize()
    
{
    // Initialize used libraries
    Testbed::initialize();
    Graphics::initialize(true);
    Testbed::addResizeCallback(&resize);
    Input::addKeyPressCallback([](Input::Key key) { if (key == Input::KEY_ESCAPE) Testbed::stop(); });
    Input::addKeyPressCallback([](Input::Key key) { if (key == Input::KEY_R) reloadShaders(); });
    Input::addMouseMoveCallback([](double dx, double dy) { camera.look(dx * SENSITIVITY, dy * SENSITIVITY); });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2, 0.2, 0.2, 1.0);
//    glGenFramebuffers(1, &fbo);
//    glGenTextures(1, &clr);
//    glGenRenderbuffers(1, &dpt);
    resize(Testbed::getScreenWidth(), Testbed::getScreenHeight());
    camera.setPosition(glm::vec3(0, 1, 5));

    // Build ocean mesh
    for (float x = -20.0f; x < 20.0f; x++)
    {
        for (float y = -20.0f; y < 20.0f; y++)
        {
            terrainData.vertices.push_back(glm::vec2(x, y));
            terrainData.vertices.push_back(glm::vec2(x, y+1.0f));        
            terrainData.vertices.push_back(glm::vec2(x + 1.0f, y));
        
            terrainData.vertices.push_back(glm::vec2(x + 1.0f, y));
            terrainData.vertices.push_back(glm::vec2(x, y + 1.0f));
            terrainData.vertices.push_back(glm::vec2(x + 1.0f, y + 1.0f));
        }
    }

    terrain = Graphics::createSurface(terrainData);
    model = Graphics::createSurface(modelData);
    ocean = Graphics::createSurface(oceanData);
    
    glActiveTexture(GL_TEXTURE1);
    heightmap = Graphics::createTexture("res/heightmap.png");
    glBindTexture(GL_TEXTURE_2D, heightmap);
    glActiveTexture(GL_TEXTURE0);

    // Load shader (reload works even the first time)
    reloadShaders(); 
}

void reloadShaders()
{   
    if (shaderModel) glDeleteProgram(shaderModel);
    if (shaderTerrain) glDeleteProgram(shaderModel);
    if (shaderOcean) glDeleteProgram(shaderOcean);
    
    printf("Loading shaders\n");
    GLuint modelVS = Graphics::loadShader("res/model.vert", GL_VERTEX_SHADER);
    GLuint terrainVS = Graphics::loadShader("res/terrain.vert", GL_VERTEX_SHADER);
    GLuint oceanVS = Graphics::loadShader("res/ocean.vert", GL_VERTEX_SHADER);
    GLuint phong = Graphics::loadShader("res/phong.frag", GL_FRAGMENT_SHADER);
    shaderModel = Graphics::createProgram({modelVS, phong});
    shaderTerrain = Graphics::createProgram({terrainVS, phong});
    shaderOcean = Graphics::createProgram({oceanVS, phong});
    glDeleteShader(modelVS);
    glDeleteShader(terrainVS);
    glDeleteShader(oceanVS);
    glDeleteShader(phong);
    
    glUseProgram(shaderModel);
    mvpModel = glGetUniformLocation(shaderModel, "MVP");
    glUseProgram(shaderTerrain);
    mvpTerrain = glGetUniformLocation(shaderTerrain, "MVP");
    glUniform1i(glGetUniformLocation(shaderTerrain, "heightmap"), 1);
    glUseProgram(shaderOcean);
    mvpOcean = glGetUniformLocation(shaderOcean, "MVP");
    glUseProgram(0);
}

void resize(int width, int height)
{
    printf("Resizing %dx%d\n", width, height);
    glViewport(0, 0, width, height);
    camera.setProjection(65.0f * 3.14159 / 180.0f, float(width) / float(height), 0.1f, 1000.0f);

//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    glBindTexture(GL_TEXTURE_2D, clr);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, clr, 0);

//    glBindRenderbuffer(GL_RENDERBUFFER, dpt);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dpt);
    
//    Testbed::assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to create FBO");
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void update(double dt)
{
    const double period = 2.0;
    static double time = 0.0;
    static int frames = 0;

    if (time >= period)
    {
        printf("Average Frame Time: %I.3fms (%.2ffps)\n", 1000.0f * time / frames, frames / time);
        time = 0;
        frames = 0;
    }
    else 
    {
        time += dt;
        frames++;
    }

    Input::poll();

    glm::vec2 mvmt(0.0f, 0.0f);
    if (Input::isKeyPressed(Input::KEY_W)) mvmt.x += 1.0f;
    if (Input::isKeyPressed(Input::KEY_S)) mvmt.x -= 1.0f;
    if (Input::isKeyPressed(Input::KEY_D)) mvmt.y += 1.0f;
    if (Input::isKeyPressed(Input::KEY_A)) mvmt.y -= 1.0f;
    if (glm::length(mvmt)) mvmt = glm::normalize(mvmt) * (float)dt;

    float up = 0.0f;
    if (Input::isKeyPressed(Input::KEY_SPACE)) up += (float)dt;
    if (Input::isKeyPressed(Input::KEY_LEFT_SHIFT)) up -= (float)dt;
    
    if (mvmt.x || mvmt.y || up) camera.move(mvmt.x, mvmt.y, up); 
    mvp = camera.getCameraMatrix();
  //glUniformMatrix4fv(mvpOcean, 1, GL_FALSE, &mvp[0][0]);
}

// TODO move this to surface
void drawSurface(const Graphics::Surface& surf, GLuint program, GLuint mvpHandle)
{
    glUseProgram(program);
    glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, &mvp[0][0]);
    Graphics::drawSurface(surf);
}

void render()
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSurface(model, shaderModel, mvpModel);
    drawSurface(terrain, shaderTerrain, mvpTerrain);
    drawSurface(ocean, shaderOcean, mvpOcean);
    Testbed::update();
}

void shutdown()
{
    Graphics::deleteSurface(model);
    Graphics::deleteSurface(terrain);
    Graphics::deleteSurface(ocean);
    glDeleteProgram(shaderModel);
    glDeleteProgram(shaderTerrain);
    glDeleteProgram(shaderOcean);
    glDeleteTextures(1, &heightmap);
//    glDeleteTextures(1, &clr);
//    glDeleteFramebuffers(1, &fbo);
//    glDeleteRenderbuffers(1, &dpt);
    Graphics::shutdown();
    Testbed::shutdown();
}
