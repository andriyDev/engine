
#pragma once

#include "std.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    ~Window();

    /*
    Constructs the window. If the window was already constructed,
    first deletes the existing window, then recreates it.
    This may also modify properties (e.g. width, height) to ensure they align with the window.
    */
    void build();

    /* Destroys the actual window but maintains all the settings so we can rebuild it. */
    void destroy();

    /* Gets the current title of the window. */
    std::string getTitle() const;
    
    /* Sets the title of the window. */
    void setTitle(std::string _windowTitle);

    /* Gets the size of the current window. */
    std::pair<uint, uint> getSize() const;

    /* Sets the size of the current window. This is a request and may not be satisfied. */
    void setSize(uint _width, uint _height);

    /* Gets the size of contents of the current viewport. May return 0,0 if the window is not built. */
    std::pair<uint, uint> getSurfaceSize() const;

    /*
    Gets the size of the borders of the window, returned left, up, right, down order.
    May return 0,0,0,0 if the window is not built.
    */
    std::tuple<uint, uint, uint, uint> getBorderSize() const;

    /* Shows the window. Does nothing if window is shown or has not been constructed. */
    void show();

    /* Hides the window. Does nothing if window is hidden or has not been constructed. */
    void hide();

    /* Binds the context of the window so OpenGL draws with this context. */
    void bindContext();

    /* Swaps buffers so we see the results of drawing. */
    void swapBuffers();

    /* Polls for events. */
    void poll();

    /* Returns true when the window is requesting to be closed. */
    bool wantsClose() const;

    /* TODO: Remove this! */
    GLFWwindow* getWindow() const { return window; };
private:
    GLFWwindow* window = nullptr; // Handle to the glfw window.

    uint width = 0; // Stores the width of the window. Updated whenever the window is resized.
    uint height = 0; // Stores the height of the window. Updated whenever the window is resized.

    std::string windowTitle; // The window title. Only updated by setTitle.
    friend void window_resized(GLFWwindow* window, int width, int height);
};