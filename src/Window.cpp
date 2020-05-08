
#include "Window.h"

#include <iostream>

Window::~Window()
{
    if(window) {
        glfwDestroyWindow(window);
    }
}

void Window::window_resized(GLFWwindow* gwindow, int width, int height)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));
    
    window->width = width;
    window->height = height;
}

void Window::key_event(GLFWwindow* gwindow, int key, int scancode, int action, int mods)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        if(action == GLFW_PRESS) {
            handler->keyPressed(key, scancode, mods);
        } else if(action == GLFW_RELEASE) {
            handler->keyReleased(key, scancode, mods);
        } else {
            handler->keyRepeated(key, scancode, mods);
        }
    }
}

void Window::char_typed(GLFWwindow* gwindow, uint character)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        handler->charTyped(character);
    }
}

void Window::mouse_btn_event(GLFWwindow* gwindow, int button, int action, int mods)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        if(action == GLFW_PRESS) {
            handler->mousePressed(button, mods);
        } else {
            handler->mouseReleased(button, mods);
        }
    }
}

void Window::mouse_move(GLFWwindow* gwindow, double x, double y)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        handler->mouseMoved(x, y);
    }
}

void Window::mouse_enter(GLFWwindow* gwindow, int entered)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        if(entered == GLFW_TRUE) {
            handler->mouseEntered();
        } else {
            handler->mouseExited();
        }
    }
}

void Window::mouse_scroll(GLFWwindow* gwindow, double x, double y)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(gwindow));

    for(WindowEventHandler* handler : window->eventHandlers) {
        handler->mouseScroll(x, y);
    }
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
    if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, Window::window_resized);
    glfwSetKeyCallback(window, Window::key_event);
    glfwSetCharCallback(window, Window::char_typed);
    glfwSetMouseButtonCallback(window, Window::mouse_btn_event);
    glfwSetCursorPosCallback(window, Window::mouse_move);
    glfwSetCursorEnterCallback(window, Window::mouse_enter);
    glfwSetScrollCallback(window, Window::mouse_scroll);
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
        glfwSwapInterval(1);
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

void Window::addEventHandler(WindowEventHandler* eventHandler)
{
    assert(eventHandler);
    eventHandlers.push_back(eventHandler);
}

void Window::removeEventHandler(WindowEventHandler* eventHandler)
{
    assert(eventHandler);
    eventHandlers.erase(std::find(eventHandlers.begin(), eventHandlers.end(), eventHandler));
}

void Window::setCursor(bool lock, bool hidden)
{
    if(!window) {
        return;
    }
    if(lock) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (hidden) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
