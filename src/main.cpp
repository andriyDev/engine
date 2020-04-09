
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/Universe.h"
#include "core/System.h"
#include "core/World.h"

#include "components/Transform.h"

class TestSystem : public System
{
public:
    virtual void frameTick(float delta, float tickPercen) override {
        //printf("Frame Tick!\n");
    }
    virtual void gameplayTick(float delta) override {
        //printf("Gameplay Tick!\n");
    }
};

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

    TransformData t(vec3(10, 0, 0), quat(vec3(3.14f/2,0,0)));
    TransformData t2(vec3(0, 23, 5), quat(vec3(3.14f/2,0,0)), vec3(1,2,3));
    printf("%s\n%s\n%s\n%s\n%s\n",
        to_string(t).c_str(),
        to_string(t2).c_str(),
        to_string(t2 * t).c_str(),
        to_string(t * t2.inverse() * t2 * t.inverse()).c_str(),
        to_string(t).c_str());

    Universe* U = Universe::init();
    U->gameplayRate = 1;

    World* w = U->addWorld(new World());
    w->addSystem(new TestSystem());

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
