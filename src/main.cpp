
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

#include "utility/Package.h"

class TestSystem : public System
{
public:
    virtual void frameTick(float delta, float tickPercen) override {
        //printf("Frame Tick!\n");
    }
    virtual void gameplayTick(float delta) override {
        Query<Transform*> mr = getWorld()->queryEntities().filter([&](Entity* e) {
            return e->findComponentByType(MESH_RENDERER_ID) && e->findComponentByType(TRANSFORM_ID);
        }).map<Transform*>([](Entity* e){ return static_cast<Transform*>(e->findComponentByType(TRANSFORM_ID)); });
        for(Transform* t : mr) {
            TransformData td = t->getRelativeTransform();
            td = TransformData(glm::vec3(0,0,0), glm::angleAxis(glm::radians(60.f) * delta, glm::vec3(0,0,1))) * td;
            t->setRelativeTransform(td);
        }
    }
};

Mesh* buildMesh() {
    Mesh* mesh = new Mesh();
    mesh->vertCount = 3;
    mesh->vertData = new Mesh::Vertex[3];
    mesh->vertData[0].position = glm::vec3(1,-1,0);
    mesh->vertData[1].position = glm::vec3(-1,-1,0);
    mesh->vertData[2].position = glm::vec3(0,1,0);

    mesh->indexCount = 3;
    mesh->indexData = new uint[3];
    mesh->indexData[0] = 0;
    mesh->indexData[1] = 1;
    mesh->indexData[2] = 2;
    return mesh;
}

std::map<uint, std::pair<WriteFcn, ReadFcn>> parsers = {
    {(uint)RenderResources::Mesh, std::make_pair(writeMesh, readMesh)},
    {(uint)RenderResources::Shader, std::make_pair(writeShader, readShader)}
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

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    PackageFile res("res.pkg", (const uchar*)"REN", &parsers);
    res.open();

    Universe* U = Universe::init();
    U->gameplayRate = 30;

    World* w = U->addWorld();
    w->addSystem<TestSystem>();
    w->addSystem<RenderSystem>(-10000);

    w->attach(U->addEntity());
    Entity* e = U->addEntity();
    w->attach(e);
    MeshRenderer* m = U->addComponent<MeshRenderer>();
    m->mesh = new RenderableMesh(res.releaseResource<Mesh>("Mesh", (uint)RenderResources::Mesh));
    std::vector<Shader*> vert_comp = {
        res.releaseResource<Shader>("vertex_basic_shader", (uint)RenderResources::Shader)
    };
    std::vector<Shader*> frag_comp = {
        res.releaseResource<Shader>("fragment_basic_shader", (uint)RenderResources::Shader)
    };
    m->material = new Material(new MaterialProgram(vert_comp, frag_comp));
    Transform* meshTransform = U->addComponent<Transform>();
    glm::vec3 a(3,0,0);
    glm::vec3 b(-10,0,0);
    meshTransform->setRelativeTransform(TransformData(a,
        glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0)),
        glm::vec3(0.6f, 0.6f, 0.6f)), true);
    
    e->attach(meshTransform)
     ->attach(m);
    Transform* camTransform = U->addComponent<Transform>();
    Entity* c = U->addEntity();
    w->attach(c);
    Camera* cam = U->addComponent<Camera>();
    c->attach(cam)
        ->attach(camTransform);
    camTransform->setRelativeTransform(TransformData(b,
        glm::quatLookAt(glm::normalize(a - b), glm::vec3(0, 0, 1))), true);
    camTransform->getGlobalTransform().transformDirection(glm::vec3(1, 0, 0));

    res.close();

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
