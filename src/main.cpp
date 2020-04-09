
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/Universe.h"

int main()
{
    GLFWwindow* window;
    if(!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1280, 720, "Title", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        return -1;
    }


    Universe* U = Universe::init();

    float previousTime = (float)glfwGetTime();

    do {
        float newTime = (float)glfwGetTime();
        float delta = newTime - previousTime;
        previousTime = newTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        U->tick(delta);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    Universe::cleanUp();

    return 0;
}
