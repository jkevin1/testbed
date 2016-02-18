#include "Testbed.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include <stdio.h>
#include <math.h>

int main(int argc, char* argv[])
{
    Testbed::initialize();
    Graphics::initialize(true);

    Graphics::printContextData();
    Input::addKeyPressCallback([](Input::Key key){ if (key == Input::KEY_ESCAPE) Testbed::stop(); });
    
    while (Testbed::running())
    {
        Input::poll();
        double t = Testbed::getTime();
        glClearColor(sin(t), cos(t), sin(cos(t)), 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        Testbed::update();
    }

    Graphics::shutdown();
    Testbed::shutdown();
}
