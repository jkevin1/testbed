#ifndef Input_HPP
#define Input_HPP

#include "Buttons.hpp"
#include <functional>

namespace Input
{
// Poll system for events
void poll();


// Accessor for key state
bool isKeyPressed(Key key);

// Accessor for mouse button state
bool isMousePressed(Button button);

// Accessors for mouse position
double getMouseX();
double getMouseY();

// TODO other callbacks and accessors
// Mouse position callback registry function (dx, dy)
void addMouseMoveCallback(std::function<void(double,double)> callback);
void addKeyPressCallback(std::function<void(Key key)> callback);
}

#endif
