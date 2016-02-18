#include "Testbed.hpp"
#include "Input.hpp"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

// Mesa currently only supports 3.3, most extensions are implemented though
#define GL_MAJOR 3
#define GL_MINOR 3
#define GL_VSYNC 0 
#define GL_DEBUG 1
// TODO implement cmake configuration for those

using namespace std;
using namespace Testbed;
using namespace Input;

//============================================================================//
// Anonymous namespace for private functions and data                         //
//============================================================================//

namespace {
    GLFWwindow* window = nullptr;

    void glfwError(int error, const char* msg)
    {
        fprintf(stderr, "GLFW Error: %s\n", msg);
        // Possibly quit from here
    }

    double mouseX, mouseY;
    vector<function<void(double,double)>> mouseMoveCallbacks;
    void glfwMouseMove(GLFWwindow*, double x, double y)
    {
        for (auto callback : mouseMoveCallbacks)
            callback(x - mouseX, y - mouseY);
        mouseX = x;
        mouseY = y;
    }

    vector<function<void(Key)>> keyPressCallbacks;
    vector<function<void(Key)>> keyReleaseCallbacks;
    void glfwKey(GLFWwindow*, int key, int, int action, int)
    {
        if (action == GLFW_PRESS)
            for (auto callback : keyPressCallbacks)
                callback(Key(key));
        else if (action == GLFW_RELEASE)
            for (auto callback : keyReleaseCallbacks)
                callback(Key(key));
    }

    vector<function<void(int,int)>> resizeCallbacks;
    void glfwResize(GLFWwindow*, int width, int height)
    {
        glfwGetCursorPos(window, &mouseX, &mouseY);
        for (auto callback : resizeCallbacks)
            callback(width, height);
    }
}

//============================================================================//
// Testbed library implementation                                            //
//============================================================================//

void Testbed::assert(bool condition, const char* msg)
{
    if (GL_DEBUG && !condition)
    {
        fprintf(stderr, "%s\n", msg);
        shutdown();
        exit(EXIT_FAILURE);
    }
}

void Testbed::initialize()
{
    printf("[Testbed] Initializing\n");
    // Dont reinitialize unnecessarily
    if (window) return;
    // TODO add mechanism to specifiy window attributes
    // this doesnt really matter since I have a tiling wm

    // Initialize GLFW
    glfwSetErrorCallback(&glfwError);
    assert(glfwInit(), "[Testbed] Failed to initialize GLFW");

    // Set window creation hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_DEBUG);

    // Find system resoluti"on
//    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
//    const GLFWvidmode& res = *glfwGetVideoMode(monitor);
    
    // Create a fullscreen window
//    window = glfwCreateWindow(res.width, res.height, "Testbed", monitor, nullptr);
    window = glfwCreateWindow(1600, 900, "Testbed", nullptr, nullptr);
    assert(window, "[Testbed] Failed to create window");

    // Initialize OpenGL context
    glfwMakeContextCurrent(window);
    glfwSwapInterval(GL_VSYNC);

    glfwSetWindowSizeCallback(window, &glfwResize);
    glfwSetCursorPosCallback(window, &glfwMouseMove);
    glfwSetKeyCallback(window, &glfwKey);

    glfwGetCursorPos(window, &mouseX, &mouseY);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Testbed::shutdown()
{
    printf("[Testbed] Shutting down\n");
    // Windows are destroyed in glfwTerminate()
    glfwTerminate();
}

bool Testbed::running()
{
    return !glfwWindowShouldClose(window);
}

void Testbed::stop()
{
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void Testbed::update()
{
    glfwSwapBuffers(window);
}

double Testbed::getTime()
{
    return glfwGetTime();
}

int Testbed::getScreenWidth()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return width;
}

int Testbed::getScreenHeight()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return height;
}

void Testbed::addResizeCallback(function<void(int,int)> callback)
{
    resizeCallbacks.push_back(callback);
}

//============================================================================//
// Input library implementation                                               //
//============================================================================//

void Input::poll()
{
    glfwPollEvents();
}

bool Input::isKeyPressed(Input::Key key)
{
    return glfwGetKey(window, key);
}

bool Input::isMousePressed(Input::Button button)
{
    return glfwGetMouseButton(window, button);
}

double Input::getMouseX()
{
    return mouseX; 
}

double Input::getMouseY()
{
    return mouseY;
}

void Input::addMouseMoveCallback(function<void(double,double)> callback)
{
    printf("[Input] Registering mouse movement callback\n");
    mouseMoveCallbacks.push_back(callback);
}

void Input::addKeyPressCallback(function<void(Key)> callback)
{
    printf("[Input] Registering key press callback\n");
    keyPressCallbacks.push_back(callback);
}
