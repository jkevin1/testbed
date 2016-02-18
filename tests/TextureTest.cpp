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

IndexedMesh<glm::vec2> terrainData = {{{2, GL_FLOAT}}, {}, {}};


IndexedMesh<glm::vec2> oceanData = {
    {{2, GL_FLOAT}},
    {glm::vec2(-20.0f, -20.0f), glm::vec2(-20.0f, 20.0f), glm::vec2(20.0f, -20.0f), glm::vec2(20.0f, 20.0f)},
    {0, 1, 2, 2, 1, 3}
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
using Graphics::Shader;
Shader modelShader;
Shader terrainShader;
Shader oceanShader;

// Uniforms TODO pack shared data into UBO
using Graphics::Variable;
Variable modelMVP;
Variable modelEye;
Variable terrainMVP;
Variable terrainEye;
Variable oceanMVP;
Variable oceanEye;
Variable oceanTime;

// Textures TODO use samplers
GLuint heightmap = 0;
GLuint bumpmap = 0;

// Global transformation matrix
glm::vec3 pos;
glm::mat4 mvp;
float totalTime = 0.0f;

int main(int argc, char* argv[])
{
    printf("%s\n", argv[0]);
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
    Input::addKeyPressCallback([](Input::Key key) { if (key == Input::KEY_1) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }); 
    Input::addKeyPressCallback([](Input::Key key) { if (key == Input::KEY_2) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }); 
    Input::addMouseMoveCallback([](double dx, double dy) { camera.look(dx * SENSITIVITY, dy * SENSITIVITY); });

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glClearColor(0.2, 0.2, 0.2, 1.0);
//    glGenFramebuffers(1, &fbo);
//    glGenTextures(1, &clr);
//    glGenRenderbuffers(1, &dpt);
    resize(Testbed::getScreenWidth(), Testbed::getScreenHeight());
    camera.setPosition(glm::vec3(0, 5, 0));

    // Fill terrain vertex buffer
    terrainData.vertices.reserve(41 * 41);
    for (float y = -20.0f; y <= 20.0f; y += 1.0f)
        for (float x = -20.0f; x <= 20.0f; x += 1.0f)
            terrainData.vertices.push_back(glm::vec2(x, y));

    terrainData.indices.reserve(40 * 40 * 2 * 3);
    for (unsigned i = 0; i < 40; i++)
    {
        for (unsigned j = 0; j < 40; j++)
        {
            const unsigned width = 41;
            unsigned index = i*width + j;
            terrainData.indices.push_back(index);
            terrainData.indices.push_back(index + width);
            terrainData.indices.push_back(index + 1);

            terrainData.indices.push_back(index + 1);
            terrainData.indices.push_back(index + width);
            terrainData.indices.push_back(index + width + 1);
        }
    }

    terrain = Graphics::createSurface(terrainData);
    model = Graphics::createSurface(modelData);
    ocean = Graphics::createSurface(oceanData);

    glActiveTexture(GL_TEXTURE1);
    heightmap = Graphics::createTexture("res/heightmap.png");
    glBindTexture(GL_TEXTURE_2D, heightmap);

    glActiveTexture(GL_TEXTURE2);
    bumpmap = Graphics::createTexture("res/bumps.png");
    glBindTexture(GL_TEXTURE_2D, bumpmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glActiveTexture(GL_TEXTURE0);

    // Load shader (reload works even the first time)
    reloadShaders(); 
}

void reloadShaders()
{   
    modelShader.release();
    terrainShader.release();
    oceanShader.release();
    
    printf("Loading shaders\n");
    GLuint modelVS = Graphics::loadShader("res/model.vert", GL_VERTEX_SHADER);
    GLuint terrainVS = Graphics::loadShader("res/terrain.vert", GL_VERTEX_SHADER);
    GLuint oceanVS = Graphics::loadShader("res/ocean.vert", GL_VERTEX_SHADER);
    GLuint phong = Graphics::loadShader("res/phong.frag", GL_FRAGMENT_SHADER);
    GLuint oceanFS = Graphics::loadShader("res/ocean.frag", GL_FRAGMENT_SHADER);
    modelShader = Graphics::createProgram({modelVS, phong});
    terrainShader = Graphics::createProgram({terrainVS, phong});
    oceanShader = Graphics::createProgram({oceanVS, oceanFS});
    glDeleteShader(modelVS);
    glDeleteShader(terrainVS);
    glDeleteShader(oceanVS);
    glDeleteShader(phong);
    glDeleteShader(oceanFS);
   
    glm::vec3 lightPos(0.0f, 3.0f, 0.0f);
    modelMVP = modelShader["MVP"];
    modelEye = modelShader["eyePos"];
    modelShader["lightPos"].set(lightPos);

    terrainMVP = terrainShader["MVP"];
    terrainEye = terrainShader["eyePos"];
    terrainShader["lightPos"].set(lightPos);
    terrainShader["heightmap"].set(1);

    oceanMVP = oceanShader["MVP"];
    oceanEye = oceanShader["eyePos"];
    oceanTime = oceanShader["time"];
    oceanShader["lightPos"].set(lightPos);
    oceanShader["normalmap"].set(2);

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
        printf("Average Frame Time: %.3fms (%.2ffps)\n", 1000.0f * time / frames, frames / time);
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
    if (glm::length(mvmt)) mvmt = glm::normalize(mvmt) * (float)dt * 2.0f;

    float up = 0.0f;
    if (Input::isKeyPressed(Input::KEY_SPACE)) up += (float)dt;
    if (Input::isKeyPressed(Input::KEY_LEFT_SHIFT)) up -= (float)dt;
    
    if (mvmt.x || mvmt.y || up) camera.move(mvmt.x, mvmt.y, up); 
    mvp = camera.getCameraMatrix();
    pos = camera.getPosition();
  
    modelMVP.set(mvp);
    modelEye.set(pos);
    terrainMVP.set(mvp);
    terrainEye.set(pos);
    oceanMVP.set(mvp);
    oceanEye.set(pos);

    totalTime += dt;
    oceanTime.set(totalTime);
}

// TODO move this to surface
void drawSurface(const Graphics::Surface& surf, const Shader& shader)
{
    shader.use();
    Graphics::drawSurface(surf);
}

void render()
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSurface(model, modelShader);
    drawSurface(terrain, terrainShader);
    drawSurface(ocean, oceanShader);
    Testbed::update();
}

void shutdown()
{
    Graphics::deleteSurface(model);
    Graphics::deleteSurface(terrain);
    Graphics::deleteSurface(ocean);
    modelShader.release();
    terrainShader.release();
    oceanShader.release();
    glDeleteTextures(1, &heightmap);
//    glDeleteTextures(1, &clr);
//    glDeleteFramebuffers(1, &fbo);
//    glDeleteRenderbuffers(1, &dpt);
    Graphics::shutdown();
    Testbed::shutdown();
}
