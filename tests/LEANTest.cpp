#include "Testbed.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "LEAN.hpp"
#include "RenderTarget.hpp"
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
    {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f)},
     {glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
     {glm::vec3( 0.0f,  1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)}}
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
RenderTarget screen;
RenderTarget secondary;

// Renderable surfaces  TODO other forms of surfaces (Instanced, Indexing, etc)
Graphics::Surface model{0};
Graphics::Surface terrain{0};
Graphics::Surface ocean{0};

// Shaders for different materials TODO pack into material object, use pipeline objects
using Graphics::Shader;
Shader modelShader;
Shader terrainShader;
Shader oceanShader;

struct WorldData {
  glm::mat4 mvp;
  glm::vec3 eye;
  float time;
  glm::vec2 dim;
};

// Uniforms TODO pack shared data into UBO
UniformBlock<WorldData> worldData;

// Textures TODO use samplers
GLuint heightmap = 0;
GLuint gradient = 0;
GLuint covariance = 0;

// Global transformation matrix
float totalTime = 0.0f;
glm::vec2 dimensions;
bool rough = true;

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
    Input::addKeyPressCallback([](Input::Key key) { if (key == Input::KEY_E) rough = !rough; oceanShader["rough"].set(rough); });
    Input::addMouseMoveCallback([](double dx, double dy) { camera.look(dx * SENSITIVITY, dy * SENSITIVITY); });

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 191.0f/255.0f, 1.0, 1.0);

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

    LEANMap lean("res/water.lean");
    glActiveTexture(GL_TEXTURE2);
    gradient = Graphics::loadLEANGradient(lean);
    glBindTexture(GL_TEXTURE_2D, gradient);
    glActiveTexture(GL_TEXTURE3);
    covariance = Graphics::loadLEANCovariance(lean);
    glBindTexture(GL_TEXTURE_2D, covariance);
    glActiveTexture(GL_TEXTURE0);

    // Load shader (reload works even the first time)
    worldData = UniformBlock<WorldData>::create();
    reloadShaders();
    resize(Testbed::getScreenWidth(), Testbed::getScreenHeight());
    camera.setPosition(glm::vec3(0, 5, 0));
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
   
    glm::vec3 light(0.0f, 10.0f, 0.0f);
    modelShader.setBinding("WorldData", worldData.getBinding());
    modelShader["light"].set(light);
    terrainShader.setBinding("WorldData", worldData.getBinding());
    terrainShader["light"].set(light);
    terrainShader["heightmap"].set(1);
    oceanShader.setBinding("WorldData", worldData.getBinding());
    oceanShader["light"].set(light);
    oceanShader["gradient"].set(2);
    oceanShader["covariance"].set(3);
    oceanShader["backBuffer"].set(4);
    oceanShader["depth"].set(5);

    glUseProgram(0);
}

void resize(int width, int height)
{
    printf("Resizing %dx%d\n", width, height);
    glViewport(0, 0, width, height);
    camera.setProjection(65.0f * 3.14159 / 180.0f, float(width) / float(height), 0.1f, 100.0f);
    dimensions = glm::vec2((float)width, (float)height);
    
    glActiveTexture(GL_TEXTURE4);
    secondary = RenderTarget::create(width, height);
    glBindTexture(GL_TEXTURE_2D, secondary.getTexture(0));
//    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE5);
    printf("Bound %u and %u as back and depth textures\n", secondary.getTexture(0), secondary.getDepthTexture());
    glBindTexture(GL_TEXTURE_2D, secondary.getDepthTexture());
//    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
}

void update(double dt)
{
    const double period = 2.0;
    static double time = 0.0;
    static int frames = 0;

    time += dt;
    frames++;
    if (time >= period)
    {
        printf("Average Frame Time: %.3fms (%.2ffps)\n", 1000.0f * time / frames, frames / time);
        time = 0;
        frames = 0;
    }

    Input::poll();
    totalTime += dt;

    glm::vec2 mvmt(0.0f, 0.0f);
    if (Input::isKeyPressed(Input::KEY_W)) mvmt.x += 1.0f;
    if (Input::isKeyPressed(Input::KEY_S)) mvmt.x -= 1.0f;
    if (Input::isKeyPressed(Input::KEY_D)) mvmt.y += 1.0f;
    if (Input::isKeyPressed(Input::KEY_A)) mvmt.y -= 1.0f;
    glGenerateMipmap(GL_TEXTURE_2D);
    if (glm::length(mvmt)) mvmt = glm::normalize(mvmt) * (float)dt * 2.0f;

    float up = 0.0f;
    if (Input::isKeyPressed(Input::KEY_SPACE)) up += (float)dt;
    if (Input::isKeyPressed(Input::KEY_LEFT_SHIFT)) up -= (float)dt;
    
    if (mvmt.x || mvmt.y || up) camera.move(mvmt.x, mvmt.y, up);
 
    WorldData* data = worldData.map();
    data->mvp = camera.getCameraMatrix();
    data->eye = camera.getPosition();
    data->time = totalTime;
    data->dim = dimensions;
    worldData.unmap();
}

// TODO move this to surface
void drawSurface(const Graphics::Surface& surf, const Shader& shader, const RenderTarget& target)
{
    target.activate();
    shader.use();
    Graphics::drawSurface(surf);
}

void render()
{
    screen.clear();
    drawSurface(model, modelShader, screen);
    glEnable(GL_CULL_FACE);
    drawSurface(terrain, terrainShader, screen);
    glDisable(GL_CULL_FACE);
    secondary.clear();
    secondary.blit(screen, Testbed::getScreenWidth(), Testbed::getScreenHeight());
    glActiveTexture(4);
    glBindTexture(GL_TEXTURE_2D, secondary.getTexture(0));
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(5);
    glBindTexture(GL_TEXTURE_2D, secondary.getDepthTexture());
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(0);
//    glGenerateTextureMipmap(secondary.getTexture(0));
//    glGenerateTextureMipmap(secondary.getDepthTexture());
    drawSurface(ocean, oceanShader, screen);
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
    secondary.release();
    Graphics::shutdown();
    Testbed::shutdown();
}
