#include "Testbed.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>

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

const Vertex verts[] = {
//  Position                Color
    {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
    {glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {glm::vec3( 0.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
};

#define SENSITIVITY 0.01f
Camera camera(0,0,0,0);

GLuint vao = 0;
GLuint vbo = 0;

GLuint shader = 0;
GLuint matrix = 0;

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
    glClearColor(0.2, 0.2, 0.2, 1.0);
    resize(Testbed::getScreenWidth(), Testbed::getScreenHeight());
    camera.setPosition(glm::vec3(0, 0, 5));

    // Load shader (reload works even the first time)
    reloadShaders();
    
    // Create vertex array
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
    // TODO wrap buffer stuff in templated functions
}

void reloadShaders()
{   
    if (shader) glDeleteProgram(shader);
    printf("Loading shaders\n");
    GLuint vert = Graphics::loadShader("res/test.vert", GL_VERTEX_SHADER);
    GLuint frag = Graphics::loadShader("res/test.frag", GL_FRAGMENT_SHADER);
    shader = Graphics::createProgram({vert, frag});
    matrix = glGetUniformLocation(shader, "MVP");
    glUseProgram(shader);
    glDeleteShader(vert);
    glDeleteShader(frag);
}

void resize(int width, int height)
{
    glViewport(0, 0, width, height);
    camera.setProjection(65.0f * 3.14159 / 180.0f, float(width) / float(height), 0.1f, 1000.0f);
}

void update(double dt)
{
    const double period = 2.0;
    static double time = 0.0;
    static int frames = 0;

    if (time >= period)
    {
        printf("Average Frame Time: %f (%.2ffps)\n", time / frames, frames / time);
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

    glUniformMatrix4fv(matrix, 1, GL_FALSE, &camera.getCameraMatrix()[0][0]);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    Testbed::update();
}

void shutdown()
{
    // TODO release resources
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);

    Graphics::shutdown();
    Testbed::shutdown();
}
