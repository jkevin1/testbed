#ifndef Testbed_HPP
#define Testbed_HPP

#include <functional>

namespace Testbed {

// Quits safely if condition is false
void assert(bool condition, const char* msg);

// Initializes the testbed application
void initialize();

// Releases resources and terminates the application
void shutdown();

// Returns true while 
bool running();

// Sets running to false
void stop();

// Process events and show the rendered frame
void update();

// Returns the time since program initialization in seconds
double getTime();

// Accessors fo the width and height of the framebuffer 
int getScreenWidth();
int getScreenHeight();

// Callback for window resize
void addResizeCallback(std::function<void(int,int)> callback);

} // Testbed

#endif // Testbed_HPP
