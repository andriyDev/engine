
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/Universe.h"
#include "core/System.h"
#include "core/World.h"
#include "core/Entity.h"

#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/Camera.h"
#include "ComponentTypes.h"
#include "renderer/RenderSystem.h"
#include "renderer/Material.h"

#include "resources/Mesh.h"
#include "resources/Shader.h"

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

Mesh* buildMesh() {
    Mesh* mesh = new Mesh();
    mesh->vertCount = 3;
    mesh->vertData = new Mesh::Vertex[3];
    mesh->vertData[0].position = vec3(-1,-1,0);
    mesh->vertData[1].position = vec3(1,-1,0);
    mesh->vertData[2].position = vec3(0,1,0);

    mesh->indexCount = 3;
    mesh->indexData = new uint[3];
    mesh->indexData[0] = 0;
    mesh->indexData[1] = 1;
    mesh->indexData[2] = 2;
    return mesh;
}

const char* basicVertShader =
"#version 330 core\n\
layout(location=0) in vec3 vert_position;\n\
uniform mat4 mvp;\n\
\n\
void main(){\n\
    gl_Position = mvp * vec4(vert_position, 1.0);\n\
}";
const char* basicFragShader =
"#version 330 core\n\
out vec3 color;\n\
\n\
void main(){\n\
    color = vec3(1,0,0);\n\
}";

Shader* shaderFromString(string str) {
    Shader* s = new Shader();
    s->code = str;
    return s;
}

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
    U->gameplayRate = 1;

    World* w = U->addWorld();
    w->addSystem<TestSystem>();
    w->addSystem<RenderSystem>(-10000);

    w->attach(U->addEntity());
    Entity* e = U->addEntity();
    w->attach(e);
    MeshRenderer* m = U->addComponent<MeshRenderer>();
    m->mesh = new RenderableMesh(buildMesh());
    vector<Shader*> vert_comp = { shaderFromString(basicVertShader) };
    vector<Shader*> frag_comp = { shaderFromString(basicFragShader) };
    m->material = new Material(new MaterialProgram(vert_comp, frag_comp));
    Transform* meshTransform = U->addComponent<Transform>();
    vec3 a(3,0,0);
    vec3 b(-3,0,0);
    meshTransform->setRelativeTransform(TransformData(a,
        quatLookAt(normalize(b - a), vec3(1, 0, 0)),
        vec3(1, 1, 1)), true);
    
    e->attach(meshTransform)
     ->attach(m);
    Transform* camTransform = U->addComponent<Transform>();
    Entity* c = U->addEntity();
    w->attach(c);
    Camera* cam = U->addComponent<Camera>();
    c->attach(cam)
        ->attach(camTransform);
    camTransform->setRelativeTransform(TransformData(b,
        quatLookAt(normalize(a - b), vec3(0, 1, 0))), true);

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
