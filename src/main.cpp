
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
#include "resources/FileResource.h"
#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/Camera.h"
#include "renderer/RenderSystem.h"
#include "renderer/Material.h"
#include "renderer/RenderableTexture.h"

#include "resources/Mesh.h"
#include "resources/Shader.h"
#include "resources/Texture.h"

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

class ApplyGravity : public Component
{
public:
    ApplyGravity() : Component(get_id(ApplyGravity)) {}
};

class GravitySystem : public System
{
public:
    virtual void gameplayTick(float delta) override {
        Query<std::shared_ptr<RigidBody>> query = getWorld()->queryComponents()
            .filter(filterByType<ApplyGravity>)
            .map_ptr<RigidBody>(mapToSibling<RigidBody>);
        for(std::shared_ptr<RigidBody> body : query) {
            body->addForce(glm::vec3(0, -10, 0) * body->mass);
        }
    }
};

void spawnBox(std::shared_ptr<World> world, glm::vec3 point)
{
    std::shared_ptr<Entity> box = world->addEntity();

    std::shared_ptr<MeshRenderer> m = box->addComponent<MeshRenderer>();
    m->mesh = 7;
    m->material = 9;
    std::shared_ptr<Transform> meshTransform = box->addComponent<Transform>();
    m->transform = meshTransform;
    meshTransform->setGlobalTransform(TransformData(point, glm::quat(0,0,0,1), glm::vec3(1, 1, 1)));
    std::shared_ptr<BoxCollider> collider = box->addComponent<BoxCollider>();
    collider->setExtents(glm::vec3(1, 1, 1) * 0.5f);
    collider->transform = meshTransform;
    std::shared_ptr<RigidBody> body = box->addComponent<RigidBody>();
    body->transform = meshTransform;
    body->mass = 10;
    body->colliders.push_back(collider);
    box->addComponent<ApplyGravity>();
}

class ControlledEntity : public Component
{
public:
    ControlledEntity() : Component(get_id(ControlledEntity)) {}
};

class CameraLookSystem : public System
{
public:
    std::weak_ptr<InputSystem> IS;
    bool* running = nullptr;
    float time = 0.f;

    virtual void frameTick(float delta) override {
        time += delta;
        std::shared_ptr<InputSystem> ISptr = IS.lock();
        if(ISptr) {
            if(ISptr->isActionDown(0, "escape", false)) {
                *running = false;
                ISptr->setTargetWindow(nullptr);
            }
        }
        Query<std::shared_ptr<Transform>> c = getWorld()->queryComponents()
            .filter(filterByType<ControlledEntity>)
            .map_ptr<Camera>(mapToSibling<Camera>)
            .map_ptr<Transform>(mapToTransform);
        for(std::shared_ptr<Transform> transform : c) {
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
        Query<std::shared_ptr<Transform>> c = getWorld()->queryComponents()
            .filter(filterByType<ControlledEntity>)
            .map_ptr<Camera>(mapToSibling<Camera>)
            .map_ptr<Transform>(mapToTransform);
        for(std::shared_ptr<Transform> transform : c) {
            if(!transform) {
                continue;
            }
            TransformData td = transform->getRelativeTransform();
            
            td.translation += (td.rotation * glm::vec3(0,0,-1)) * ISptr->getActionValue(0, "forward", true) * delta * 5.f;
            td.translation += (td.rotation * glm::vec3(-1,0,0)) * ISptr->getActionValue(0, "left", true) * delta * 5.f;
            td.translation += (td.rotation * glm::vec3(0,1,0)) * ISptr->getActionValue(0, "up", true) * delta * 5.f;
            transform->setRelativeTransform(td);
        }
    }
};

class BoxSpawner : public System
{
public:
    std::weak_ptr<InputSystem> IS;
    bool down = false;

    virtual void gameplayTick(float delta) override {
        std::shared_ptr<InputSystem> ISptr = IS.lock();
        if(ISptr->isActionDown(0, "lmb", true)) {
            // Workaround until we get isActionPressed working properly.
            if(down) {
                return;
            }
            down = true;
            Query<std::shared_ptr<Transform>> t = getWorld()->queryComponents()
                .filter(filterByType<ControlledEntity>)
                .map_ptr<Camera>(mapToSibling<Camera>)
                .map_ptr<Transform>(mapToTransform);
            for(std::shared_ptr<Transform> transform : t) {
                TransformData td = transform->getGlobalTransform();
                spawnBox(getWorld(), td.transformPoint(glm::vec3(0, 0, -5)));
            }
        }
        else {
            down = false;
        }
    }
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

    ResourceLoader& loader = ResourceLoader::get();
    {
        loader.addAssetType(typeid(Mesh), FileResource::build<Mesh>);
        loader.addAssetType(typeid(Shader), FileResource::build<Shader>);
        loader.addAssetType(typeid(Texture), FileResource::build<Texture>);
        loader.addAssetType(typeid(RenderableMesh), RenderableMesh::build);
        loader.addAssetType(typeid(RenderableTexture), RenderableTexture::build);
        loader.addAssetType(typeid(MaterialProgram), MaterialProgram::build);
        loader.addAssetType(typeid(Material), Material::build);
    }
    std::shared_ptr<Mesh> box = Mesh::makeBox(glm::vec3(0.5f, 0.5f, 0.5f));
    {
        loader.addResource(1, box);

        loader.addAssetData(2, typeid(Mesh), Mesh::createAssetData("mesh.mpk"));
        loader.addAssetData(3, typeid(Texture), Texture::createAssetData("texture.tpk"));
        loader.addAssetData(4, typeid(Shader), Shader::createAssetData("res/basic_shader.v"));
        loader.addAssetData(5, typeid(Shader), Shader::createAssetData("res/basic_shader.f"));
        loader.addAssetData(6, typeid(MaterialProgram), MaterialProgram::createAssetData({4}, {5}));
        loader.addAssetData(7, typeid(RenderableMesh), RenderableMesh::createAssetData(1));
        {
            auto textureData = RenderableTexture::createAssetData(3);
            textureData->wrapU = RenderableTexture::Clamp;
            textureData->wrapV = RenderableTexture::Clamp;
            loader.addAssetData(8, typeid(RenderableTexture), textureData);
        }
        {
            auto materialData = Material::createAssetData(6);
            materialData->setTexture("tex", 8);
            materialData->setVec3Property("albedo", glm::vec3(1,1,1));
            loader.addAssetData(9, typeid(Material), materialData);
        }
    }

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
        IS->createAction(0, "lmb");
        IS->addActionMouseBind(0, "lmb", GLFW_MOUSE_BUTTON_LEFT);
        IS->createAction(0, "rmb");
        IS->addActionMouseBind(0, "rmb", GLFW_MOUSE_BUTTON_RIGHT);
        IS->setCursor(true, true);

        std::shared_ptr<CameraLookSystem> CLS = w->addSystem<CameraLookSystem>(10);
        CLS->IS = IS;
        CLS->running = &running;

        w->addSystem<GravitySystem>(5);

        std::shared_ptr<PhysicsSystem> Physics = w->addSystem<PhysicsSystem>(0);
        Physics->setGravity(glm::vec3(0,0,0));

        w->addSystem<BoxSpawner>(6)->IS = IS;

        w->addEntity();

        glm::vec3 floorPos(0,0,0);
        glm::vec3 camPos(0,5,5);
        glm::vec3 boxPos(0,5,0);

        std::shared_ptr<Entity> floor = w->addEntity();
        {
            std::shared_ptr<MeshRenderer> m = floor->addComponent<MeshRenderer>();
            m->mesh = 7;
            m->material = 9;
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

        spawnBox(w, boxPos);
        
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
            camera->addComponent<ControlledEntity>();
        }
    }

    float previousTime = (float)glfwGetTime();
    float fpsTime = 0;

    do {
        float newTime = (float)glfwGetTime();
        float delta = newTime - previousTime;
        fpsTime += delta;
        previousTime = newTime;

        loader.loadStep();

        if(fpsTime >= 1) {
            fpsTime -= 1;
            std::cout << "FPS: " << (1.0f / delta) << std::endl;
        }

        U.tick(delta);
    } while(running && !window.wantsClose());

    return 0;
}
