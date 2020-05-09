
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
#include "resources/Texture.h"

#include "utility/Package.h"

#include "Window.h"
#include "InputSystem.h"

#include <glm/gtx/string_cast.hpp>

namespace glm
{
    vec3 toAxisRotator(quat q) {
        vec3 f = q * vec3(0, 0, -1);
        float pitch = (float)asin(f.y);
        f.y = 0;
        f = normalize(f);
        float yaw = atan2f(-f.x, -f.z);
        return vec3(yaw, pitch, 0) * 180.f / 3.14159f;
        /*
        return eulerAngles(q) * 180.f / 3.14159f;
        vec3 f = q * vec3(0, 0, 1);
        vec3 fp = vec3(f.x, 0, f.z);
        float pitch = (float)asin(f.y);
        fp = normalize(fp);
        float yaw = atan2f(fp.x, fp.z);
        q = angleAxis(pitch, vec3(-1, 0, 0)) * angleAxis(yaw, vec3(0, -1, 0)) * q;
        float roll = angle(q);
        return eulerAngles(q);//vec3(yaw, pitch, 0) * 180.f / 3.14159f;*/
    }

    quat fromAxisRotator(vec3 v) {
        
        v *= 3.14159 / 180.f;
        return angleAxis(v.x, vec3(0, 1, 0))
            * angleAxis(v.y, vec3(1, 0, 0))
            * angleAxis(v.z, vec3(0, 0, -1));
    }
};

inline float clamp(float a, float m, float M) {
    if(a < m) { a = m; }
    if(a > M) { a = M; }
    return a;
}

class TestSystem : public System
{
public:
    InputSystem* IS = nullptr;
    bool* running = nullptr;
    std::shared_ptr<Material> A;
    std::shared_ptr<Material> B;
    float time = 0.f;

    virtual void frameTick(float delta, float tickPercent) override {
        time += delta;
        Query<MeshRenderer*> mrs = getWorld()->queryComponents().filter(filterByTypeId(MESH_RENDERER_ID)).cast<MeshRenderer*>();
        for(MeshRenderer* mr : mrs) {
            mr->material = fmod(time, 2.f) < 1.f ? A : B;
        }

        if(IS) {
            if(IS->isActionDown(0, "escape", false)) {
                *running = false;
            }
        }
    }
    virtual void gameplayTick(float delta) override {
        Query<Camera*> c = getWorld()->queryComponents().filter(filterByTypeId(CAMERA_ID)).cast<Camera*>();
        for(Camera* cam : c) {
            Transform* transform = Transform::getComponentTransform(cam);
            TransformData td = transform->getRelativeTransform();
            //std::cout << glm::to_string(td.rotation * glm::vec3(0,0,1)) << std::endl;
            td.translation += (td.rotation * glm::vec3(0,0,-1)) * IS->getActionValue(0, "forward", true) * delta * 30.f;
            td.translation += (td.rotation * glm::vec3(-1,0,0)) * IS->getActionValue(0, "left", true) * delta * 30.f;
            td.translation += (td.rotation * glm::vec3(0,1,0)) * IS->getActionValue(0, "up", true) * delta * 30.f;
            glm::vec3 eulerRot = glm::toAxisRotator(td.rotation);
            std::cout << glm::to_string(eulerRot) << std::endl;
            eulerRot.x -= IS->getActionValue(0, "lookYaw", true) * 0.2f;
            eulerRot.y = clamp(eulerRot.y - IS->getActionValue(0, "lookPitch", true) * 0.2f, -89.f, 89.f);
            td.rotation = glm::fromAxisRotator(eulerRot);
            transform->setRelativeTransform(td);
        }
    }
};

std::map<uint, std::tuple<WriteFcn, ReadFcn, ReadIntoFcn>> parsers = {
    {(uint)FileRenderResources::Mesh, std::make_tuple(writeMesh, readMesh, readIntoMesh)},
    {(uint)FileRenderResources::Shader, std::make_tuple(writeShader, readShader, readIntoShader)},
    {(uint)FileRenderResources::Texture, std::make_tuple(writeTexture, readTexture, readIntoTexture)}
};

int main()
{
    Window window;
    window.setSize(1280, 720);
    window.setTitle("Title");
    window.build();

    window.bindContext();

    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        return -1;
    }

    std::shared_ptr<PackageFile> res = std::make_shared<PackageFile>("res.pkg", (const uchar*)"REN", &parsers);
    res->open();

    ResourceLoader loader;
    {
        loader.addResource("Mesh", std::make_shared<MeshBuilder>("Mesh", res));
        auto rmb = std::make_shared<RenderableMeshBuilder>();
        rmb->sourceMesh = "Mesh";
        loader.addResource("RenderMesh", rmb);

        loader.addResource("VShader", std::make_shared<ShaderBuilder>("vertex_basic_shader", res));
        loader.addResource("FShader", std::make_shared<ShaderBuilder>("fragment_basic_shader", res));
        auto mpb = std::make_shared<MaterialProgramBuilder>();
        mpb->vertexComponents.push_back("VShader");
        mpb->fragmentComponents.push_back("FShader");
        loader.addResource("Program", mpb);
    }

    loader.initLoad();
    loader.beginLoad();

    std::shared_ptr<Material> m1 = std::make_shared<Material>(
        loader.getResource<MaterialProgram>("Program", (uint)RenderResources::MaterialProgram));
    m1->setVec3Property("albedo", glm::vec3(0.361f, 0.620f, 0.322f));
    std::shared_ptr<Material> m2 = std::make_shared<Material>(m1);
    m2->setVec3Property("albedo", glm::vec3(0.1f, 0.1f, 0.95f));

    Universe* U = Universe::init();
    U->gameplayRate = 30;

    World* w = U->addWorld();
    RenderSystem* RS = w->addSystem<RenderSystem>(-10000);
    RS->targetWindow = &window;
    InputSystem* IS = w->addSystem<InputSystem>();
    IS->setTargetWindow(&window);
    IS->setControlSetCount(1);
    IS->createAction(0, "escape");
    IS->addActionKeyBind(0, "escape", GLFW_KEY_ESCAPE, false, false, false);
    IS->createAction(0, "lookYaw");
    IS->addActionSpecialMouseBind(0, "lookYaw", MOUSE_X_POS, 1.0f);
    IS->addActionSpecialMouseBind(0, "lookYaw", MOUSE_X_NEG, -1.0f);
    IS->createAction(0, "lookPitch");
    IS->addActionSpecialMouseBind(0, "lookPitch", MOUSE_Y_POS, 1.0f);
    IS->addActionSpecialMouseBind(0, "lookPitch", MOUSE_Y_NEG, -1.0f);
    IS->createAction(0, "forward");
    IS->addActionKeyBind(0, "forward", 'W', false, false, false);
    IS->addActionKeyBind(0, "forward", 'S', false, false, false, -1.0f);
    IS->createAction(0, "left");
    IS->addActionKeyBind(0, "left", 'A', false, false, false);
    IS->addActionKeyBind(0, "left", 'D', false, false, false, -1.0f);
    IS->createAction(0, "up");
    IS->addActionKeyBind(0, "up", ' ', false, false, false);
    IS->addActionKeyBind(0, "up", GLFW_KEY_LEFT_SHIFT, false, false, false, -1.0f);
    IS->setCursor(true, true);

    bool running = true;
    TestSystem* TS = w->addSystem<TestSystem>();
    TS->A = m1;
    TS->B = m2;
    TS->IS = IS;
    TS->running = &running;

    w->attach(U->addEntity());
    Entity* e = U->addEntity();
    w->attach(e);
    MeshRenderer* m = U->addComponent<MeshRenderer>();
    m->mesh = loader.getResource<RenderableMesh>("RenderMesh", (uint)RenderResources::RenderableMesh);
    m->material = nullptr;
    Transform* meshTransform = U->addComponent<Transform>();
    glm::vec3 a(0,0,-10);
    glm::vec3 b(0,0,0);
    meshTransform->setRelativeTransform(TransformData(a,
        glm::angleAxis(glm::radians(-90.f), glm::vec3(1, 0, 0)),
        glm::vec3(0.6f, 0.6f, 0.6f)), true);
    
    e->attach(meshTransform)
     ->attach(m);
    Transform* camTransform = U->addComponent<Transform>();
    Entity* c = U->addEntity();
    w->attach(c);
    Camera* cam = U->addComponent<Camera>();
    c->attach(cam)
        ->attach(camTransform);
    camTransform->setRelativeTransform(TransformData(b), true);

    res->close();

    float previousTime = (float)glfwGetTime();
    float fpsTime = 0;

    do {
        float newTime = (float)glfwGetTime();
        float delta = newTime - previousTime;
        fpsTime += delta;
        previousTime = newTime;

        loader.poll();

        if(fpsTime >= 1) {
            fpsTime -= 1;
            std::cout << "FPS: " << (1.0f / delta) << std::endl;
        }

        U->tick(delta);
    } while(running && !window.wantsClose());

    Universe::cleanUp();

    return 0;
}
