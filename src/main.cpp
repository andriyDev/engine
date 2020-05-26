
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
#include "renderer/RenderableTexture.h"

#include "resources/Mesh.h"
#include "resources/Shader.h"
#include "resources/Texture.h"

#include "utility/Package.h"

#include "Window.h"
#include "InputSystem.h"
#include "physics/PhysicsSystem.h"
#include "physics/StaticBody.h"
#include "physics/BoxCollider.h"
#include "physics/SphereCollider.h"
#include "physics/KinematicBody.h"
#include "physics/RigidBody.h"

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
    std::weak_ptr<InputSystem> IS;
    bool* running = nullptr;
    std::shared_ptr<Material> A;
    std::shared_ptr<Material> B;
    std::shared_ptr<Transform> moveTarget;
    std::shared_ptr<BoxCollider> boxTarget;
    std::shared_ptr<RigidBody> rbTarget;
    float time = 0.f;

    virtual void frameTick(float delta) override {
        time += delta;
        /*
        Query<std::shared_ptr<MeshRenderer>> mrs = getWorld()->queryComponents()
            .filter(filterByTypeId(MESH_RENDERER_ID))
            .cast_ptr<MeshRenderer>();
        for(std::shared_ptr<MeshRenderer> mr : mrs) {
            mr->material = A;//fmod(time, 2.f) < 1.f ? A : B;
        }*/
        std::shared_ptr<InputSystem> ISptr = IS.lock();
        if(ISptr) {
            if(ISptr->isActionDown(0, "escape", false)) {
                *running = false;
                ISptr->setTargetWindow(nullptr);
            }
        }
        Query<std::shared_ptr<Camera>> c = getWorld()->queryComponents()
            .filter(filterByTypeId(CAMERA_ID))
            .cast_ptr<Camera>();
        for(std::shared_ptr<Camera> cam : c) {
            std::shared_ptr<Transform> transform = cam->getTransform();
            if(!transform) {
                continue;
            }
            TransformData td = transform->getRelativeTransform();
            glm::vec3 eulerRot = glm::toAxisRotator(td.rotation);
            eulerRot.x -= ISptr->getActionValue(0, "lookYaw", false) * 0.1f;
            eulerRot.y = clamp(eulerRot.y - ISptr->getActionValue(0, "lookPitch", false) * 0.1f, -89.f, 89.f);
            td.rotation = glm::fromAxisRotator(eulerRot);
            transform->setRelativeTransform(td);
        }
    }
    virtual void gameplayTick(float delta) override {
        std::shared_ptr<InputSystem> ISptr = IS.lock();
        Query<std::shared_ptr<Camera>> c = getWorld()->queryComponents()
            .filter(filterByTypeId(CAMERA_ID))
            .cast_ptr<Camera>();
        for(std::shared_ptr<Camera> cam : c) {
            std::shared_ptr<Transform> transform = cam->getTransform();
            if(!transform) {
                continue;
            }
            TransformData td = transform->getRelativeTransform();
            
            td.translation += (td.rotation * glm::vec3(0,0,-1)) * ISptr->getActionValue(0, "forward", true) * delta * 5.f;
            td.translation += (td.rotation * glm::vec3(-1,0,0)) * ISptr->getActionValue(0, "left", true) * delta * 5.f;
            td.translation += (td.rotation * glm::vec3(0,1,0)) * ISptr->getActionValue(0, "up", true) * delta * 5.f;
            transform->setRelativeTransform(td);
        }
        if(rbTarget) {
            rbTarget->addImpulse(glm::vec3(0, -9.81f, 0));
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
        loader.addResource("Box", Mesh::makeBox(glm::vec3(0.5f,0.5f,0.5f)));
        loader.addResource("Mesh", std::make_shared<MeshBuilder>("Mesh", res));
        auto rmb = std::make_shared<RenderableMeshBuilder>();
        rmb->sourceMesh = "Box";
        loader.addResource("RenderMesh", rmb);

        loader.addResource("Texture", std::make_shared<TextureBuilder>("EarthTexture", res));
        auto rtb = std::make_shared<RenderableTextureBuilder>();
        rtb->sourceTexture = "Texture";
        rtb->wrapU = RenderableTextureBuilder::Clamp;
        rtb->wrapV = RenderableTextureBuilder::Clamp;
        loader.addResource("RenderTexture", rtb);

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
    m1->setVec3Property("albedo", glm::vec3(1, 1, 1));
    std::shared_ptr<Material> m2 = std::make_shared<Material>(
        loader.getResource<MaterialProgram>("Program", (uint)RenderResources::MaterialProgram));
    m2->setVec3Property("albedo", glm::vec3(0.1f, 0.1f, 0.95f));
    m1->setTexture("tex", loader.getResource<RenderableTexture>("RenderTexture",
        (uint)RenderResources::RenderableTexture));
    m2->setTexture("tex", loader.getResource<RenderableTexture>("RenderTexture",
        (uint)RenderResources::RenderableTexture));

    res->close();

    Universe U;
    U.gameplayRate = 60;
    bool running = true;

    {
        std::shared_ptr<World> w = U.addWorld();
        std::shared_ptr<RenderSystem> RS = w->addSystem<RenderSystem>(-10000);
        RS->targetWindow = &window;
        std::shared_ptr<InputSystem> IS = w->addSystem<InputSystem>(20);
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

        std::shared_ptr<TestSystem> TS = w->addSystem<TestSystem>(10);
        TS->A = m1;
        TS->B = m2;
        TS->IS = IS;
        TS->running = &running;

        std::shared_ptr<PhysicsSystem> Physics = w->addSystem<PhysicsSystem>(0);
        Physics->setGravity(glm::vec3(0,0,0));

        w->addEntity();

        glm::vec3 floorPos(0,0,0);
        glm::vec3 camPos(0,5,5);
        glm::vec3 boxPos(0,5,0);

        std::shared_ptr<Entity> floor = w->addEntity();
        {
            std::shared_ptr<MeshRenderer> m = floor->addComponent<MeshRenderer>();
            m->mesh = loader.getResource<RenderableMesh>("RenderMesh", (uint)RenderResources::RenderableMesh);
            m->material = m1;
            std::shared_ptr<Transform> meshTransform = floor->addComponent<Transform>();
            m->transform = meshTransform;
            meshTransform->setGlobalTransform(TransformData(floorPos, glm::quat(0,0,0,1), glm::vec3(15, 1, 15)));
            std::shared_ptr<BoxCollider> collider = floor->addComponent<BoxCollider>();
            collider->setExtents(glm::vec3(15, 1, 15) * 0.5f);
            collider->transform = meshTransform;
            std::shared_ptr<StaticBody> body = floor->addComponent<StaticBody>();
            body->transform = meshTransform;
            body->colliders.push_back(collider);
        }

        std::shared_ptr<Entity> box = w->addEntity();
        {
            std::shared_ptr<MeshRenderer> m = box->addComponent<MeshRenderer>();
            m->mesh = loader.getResource<RenderableMesh>("RenderMesh", (uint)RenderResources::RenderableMesh);
            m->material = m1;
            std::shared_ptr<Transform> meshTransform = box->addComponent<Transform>();
            m->transform = meshTransform;
            meshTransform->setGlobalTransform(TransformData(boxPos, glm::quat(0,0,0,1), glm::vec3(1, 1, 1)));
            std::shared_ptr<BoxCollider> collider = floor->addComponent<BoxCollider>();
            collider->setExtents(glm::vec3(1, 1, 1) * 0.5f);
            collider->transform = meshTransform;
            std::shared_ptr<RigidBody> body = box->addComponent<RigidBody>();
            body->transform = meshTransform;
            body->mass = 10;
            body->colliders.push_back(collider);
            
            TS->moveTarget = meshTransform;
            TS->boxTarget = collider;
            TS->rbTarget = body;
        }
        
        std::shared_ptr<Entity> camera = w->addEntity();
        {
            std::shared_ptr<Transform> camTransform = camera->addComponent<Transform>();
            std::shared_ptr<Camera> cam = camera->addComponent<Camera>();
            cam->transform = camTransform;
            camTransform->setRelativeTransform(TransformData(camPos));
            std::shared_ptr<SphereCollider> collider = camera->addComponent<SphereCollider>();
            collider->transform = camTransform;
            collider->setRadius(0.5f);
            std::shared_ptr<KinematicBody> body = camera->addComponent<KinematicBody>();
            body->transform = camTransform;
            body->colliders.push_back(collider);
        }
    }

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

        U.tick(delta);
    } while(running && !window.wantsClose());

    return 0;
}
