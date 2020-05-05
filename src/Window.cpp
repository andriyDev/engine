
#include "Window.h"

Window::~Window()
{
    if(window) {
        glfwDestroyWindow(window);
    }
}

void window_resized(GLFWwindow* gwindow, int width, int height)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));
    
    window->width = width;
    window->height = height;
}

void Window::build()
{
    if(!glfwInit()) {
        throw "Failed to initialize GLFW.";
    }

    destroy(); // Ensure we don't have an existing window.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, windowTitle.c_str(), NULL, NULL);
    if(!window) {
        throw "Failed to construct window.";
    }
    glfwSetWindowUserPointer(window, this);
}

void Window::destroy()
{
    if(window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
}

std::string Window::getTitle() const
{
    return windowTitle;
}

void Window::setTitle(std::string _windowTitle)
{
    windowTitle = _windowTitle;
    if(window) {
        glfwSetWindowTitle(window, windowTitle.c_str());
    }
}

std::pair<uint, uint> Window::getSize() const
{
    return std::make_pair(width, height);
}

void Window::setSize(uint _width, uint _height)
{
    width = _width;
    height = _height;
    if(window) {
        glfwSetWindowSize(window, width, height);
    }
}

std::pair<uint, uint> Window::getSurfaceSize() const
{
    if(!window) {
        return std::make_pair(0,0);
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

std::tuple<uint, uint, uint, uint> Window::getBorderSize() const
{
    if(!window) {
        return std::make_tuple(0,0,0,0);
    }
    int l, r, u, d;
    glfwGetWindowFrameSize(window, &l, &u, &r, &d);
    return std::make_tuple(l, u, r, d);
}

void Window::show()
{
    if(window) {
        glfwShowWindow(window);
    }
}

void Window::hide()
{
    if(window) {
        glfwHideWindow(window);
    }
}

void Window::bindContext()
{
    if(window) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
    }
}

void Window::swapBuffers()
{
    if(window) {
        glfwSwapBuffers(window);
    }
}

void Window::poll()
{
    if(window) {
        glfwPollEvents();
    }
}

bool Window::wantsClose() const
{
    if(window) {
        return glfwWindowShouldClose(window);
    }
    return false;
}
