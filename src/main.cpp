
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

class TestSystem : public System
{
public:
    std::weak_ptr<InputSystem> IS;
    bool* running = nullptr;
    ResourceRef<Material> A;
    ResourceRef<Material> B;
    float time = 0.f;

    virtual void frameTick(float delta) override {
        time += delta;
        Query<std::shared_ptr<MeshRenderer>> mrs = getWorld()->queryComponents()
            .filter(filterByTypeId(MESH_RENDERER_ID))
            .cast_ptr<MeshRenderer>();
        A.resolve(Immediate);
        B.resolve(Immediate);
        for(std::shared_ptr<MeshRenderer> mr : mrs) {
            mr->material = A;//fmod(time, 2.f) < 1.f ? A : B;
        }
        std::shared_ptr<InputSystem> ISptr = IS.lock();
        if(ISptr) {
            if(ISptr->isActionDown(0, "escape", false)) {
                *running = false;
                ISptr->setTargetWindow(nullptr);
            }
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
            glm::vec3 eulerRot = glm::toAxisRotator(td.rotation);
            eulerRot.x -= ISptr->getActionValue(0, "lookYaw", true);
            eulerRot.y = clamp(eulerRot.y - ISptr->getActionValue(0, "lookPitch", true), -89.f, 89.f);
            td.rotation = glm::fromAxisRotator(eulerRot);
            transform->setRelativeTransform(td);
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
    {
        loader.addResource(1, Mesh::makeBox(glm::vec3(15, 1, 15)));

        loader.addAssetData(2, typeid(Mesh), Mesh::createAssetData("mesh.mpk"));
        loader.addAssetData(3, typeid(Texture), Texture::createAssetData("texture.tpk"));
        loader.addAssetData(4, typeid(Shader), Shader::createAssetData("res/basic_shader.v"));
        loader.addAssetData(5, typeid(Shader), Shader::createAssetData("res/basic_shader.f"));
        loader.addAssetData(6, typeid(MaterialProgram), MaterialProgram::createAssetData({4}, {5}));
        loader.addAssetData(7, typeid(RenderableMesh), RenderableMesh::createAssetData(2));
        {
            auto textureData = RenderableTexture::createAssetData(3);
            textureData->wrapU = RenderableTexture::Clamp;
            textureData->wrapV = RenderableTexture::Clamp;
            loader.addAssetData(8, typeid(RenderableTexture), textureData);
        }
        {
            auto materialData = Material::createAssetData(8);
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
        std::shared_ptr<InputSystem> IS = w->addSystem<InputSystem>();
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

        std::shared_ptr<TestSystem> TS = w->addSystem<TestSystem>();
        TS->A = 8;
        TS->B = 0;
        TS->IS = IS;
        TS->running = &running;

        w->addEntity();
        std::shared_ptr<Entity> e = w->addEntity();
        std::shared_ptr<MeshRenderer> m = e->addComponent<MeshRenderer>();
        m->mesh = 7;
        m->material = 0;
        std::shared_ptr<Transform> meshTransform = e->addComponent<Transform>();
        m->transform = meshTransform;
        glm::vec3 a(0,0,0);
        glm::vec3 b(0,5,0);
        
        std::shared_ptr<Entity> c = w->addEntity();
        std::shared_ptr<Transform> camTransform = c->addComponent<Transform>();
        std::shared_ptr<Camera> cam = c->addComponent<Camera>();
        cam->transform = camTransform;
        camTransform->setRelativeTransform(TransformData(b));
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
