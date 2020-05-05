
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/Universe.h"
#include "core/System.h"
#include "core/World.h"
#include "core/Entity.h"

#include "std.h"

#include "resources/ResourceLoader.h"
#include "resources/FileResourceBuilder.h"
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
    std::shared_ptr<Material> A;
    std::shared_ptr<Material> B;
    float time = 0.f;

    virtual void frameTick(float delta, float tickPercen) override {
        time += delta;
        Query<MeshRenderer*> mrs = getWorld()->queryComponents().filter(filterByTypeId(MESH_RENDERER_ID)).cast<MeshRenderer*>();
        for(MeshRenderer* mr : mrs) {
            std::cout << (fmod(time, 2.f) < 1.f ? "A" : "B") << std::endl;
            mr->material = fmod(time, 2.f) < 1.f ? A : B;
        }
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

std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>> parsers = {
    {(uint)FileRenderResources::Mesh, std::make_tuple(writeMesh, readMesh, readIntoMesh)},
    {(uint)FileRenderResources::Shader, std::make_tuple(writeShader, readShader, readIntoShader)}
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

    std::shared_ptr<PackageFile> res = std::make_shared<PackageFile>("res.pkg", (const uchar*)"REN", &parsers);
    res->open();

    ResourceLoader loader;
    {
        loader.addResource("Mesh", std::make_shared<FileResourceBuilder<Mesh>>(
            (uint)RenderResources::Mesh, res, "Mesh", (uint)FileRenderResources::Mesh
        ));
        auto rmb = std::make_shared<RenderableMeshBuilder>();
        rmb->sourceMesh = "Mesh";
        loader.addResource("RenderMesh", rmb);

        loader.addResource("VShader", std::make_shared<FileResourceBuilder<Shader>>(
            (uint)RenderResources::Shader, res, "vertex_basic_shader", (uint)FileRenderResources::Shader
        ));
        loader.addResource("FShader", std::make_shared<FileResourceBuilder<Shader>>(
            (uint)RenderResources::Shader, res, "fragment_basic_shader", (uint)FileRenderResources::Shader
        ));
        auto mpb = std::make_shared<MaterialProgramBuilder>();
        mpb->vertexComponents.push_back("VShader");
        mpb->fragmentComponents.push_back("FShader");
        loader.addResource("Program", mpb);

        auto mb1 = std::make_shared<MaterialBuilder>();
        mb1->materialProgram = "Program";
        loader.addResource("Material1", mb1);
        mb1->setVec3Property("albedo", glm::vec3(0.361f, 0.620f, 0.322f));
        auto mb2 = std::make_shared<MaterialBuilder>();
        mb2->materialProgram = "Program";
        loader.addResource("Material2", mb2);
        mb2->setVec3Property("albedo", glm::vec3(0.1f, 0.1f, 0.95f));
    }

    loader.initLoad();
    loader.beginLoad();

    Universe* U = Universe::init();
    U->gameplayRate = 30;

    World* w = U->addWorld();
    TestSystem* TS = w->addSystem<TestSystem>();
    TS->A = loader.getResource<Material>("Material1", (uint)RenderResources::Material);
    TS->B = loader.getResource<Material>("Material2", (uint)RenderResources::Material);
    w->addSystem<RenderSystem>(-10000);

    w->attach(U->addEntity());
    Entity* e = U->addEntity();
    w->attach(e);
    MeshRenderer* m = U->addComponent<MeshRenderer>();
    m->mesh = loader.getResource<RenderableMesh>("RenderMesh", (uint)RenderResources::RenderableMesh);
    m->material = loader.getResource<Material>("Material2", (uint)RenderResources::Material);
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

    res->close();

    float previousTime = (float)glfwGetTime();

    do {
        float newTime = (float)glfwGetTime();
        float delta = newTime - previousTime;
        previousTime = newTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        loader.poll();

        U->tick(delta);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    Universe::cleanUp();

    return 0;
}
