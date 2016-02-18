#include "Testbed.hpp"

#include "Graphics.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Models.hpp"
#include "Texture.hpp"
#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>
#undef assert

void initialize();
void resize(int x, int y);
void update(double dt);
void render();
void shutdown();

#define SENSITIVITY 0.01f
Camera camera(0,0,0,0);

// Textures TODO use samplers
GLuint heightmap;

// Global transformation matrix
glm::mat4 mvp;

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
    camera.setPosition(glm::vec3(0, 1, 5));

    Models::initialize();
    Models::update(camera.getCameraMatrix(), camera.getPosition());

    // Fill terrain vertex buffer
    glActiveTexture(GL_TEXTURE1);
    heightmap = Graphics::createTexture("res/heightmap.png");
    glBindTexture(GL_TEXTURE_2D, heightmap);
    glActiveTexture(GL_TEXTURE0);
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
    if (glm::length(mvmt)) mvmt = glm::normalize(mvmt) * (float)dt;

    float up = 0.0f;
    if (Input::isKeyPressed(Input::KEY_SPACE)) up += (float)dt;
    if (Input::isKeyPressed(Input::KEY_LEFT_SHIFT)) up -= (float)dt;
    
    if (mvmt.x || mvmt.y || up) camera.move(mvmt.x, mvmt.y, up); 
    Models::update(camera.getCameraMatrix(), camera.getPosition());
}

void render()
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Models::draw();
    Testbed::update();
}

void shutdown()
{
    Models::release();
    glDeleteTextures(1, &heightmap);
//    glDeleteTextures(1, &clr);
//    glDeleteFramebuffers(1, &fbo);
//    glDeleteRenderbuffers(1, &dpt);
    Graphics::shutdown();
    Testbed::shutdown();
}
