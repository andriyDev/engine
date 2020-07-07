
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "core/Universe.h"
#include "core/System.h"
#include "core/World.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "std.h"

#include "resources/ResourceLoader.h"
#include "resources/FileResource.h"
#include "components/Transform.h"
#include "renderer/MeshRenderer.h"
#include "renderer/Camera.h"
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
#include "physics/Trigger.h"
#include "physics/RigidBody.h"

#include "ui/UISystem.h"
#include "ui/ContainerLayouts.h"
#include "ui/Box.h"
#include "ui/Button.h"
#include "ui/Text.h"
#include "ui/Image.h"

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

class GravityRegion : public Component
{
public:
    GravityRegion() : Component(get_id(GravityRegion)) {}
};

class GravitySystem : public System
{
public:
    virtual void gameplayTick(float delta) override {
        auto regionQuery = getWorld()->queryComponents(get_id(GravityRegion))
            .map_ptr<Trigger>(mapToSibling<Trigger>)
            .map_group_ptr<CollisionObject>([](shared_ptr<Trigger> t){ return t->getOverlaps(); });
        auto bodyQuery = getWorld()->queryComponents(get_id(ApplyGravity))
            .map_ptr<RigidBody>(mapToSibling<RigidBody>);
        for(shared_ptr<RigidBody> body : bodyQuery) {
            if(regionQuery.contains(body)) {
                body->addForce(vec3(0, -10, 0) * body->mass);
            }
        }
    }
};

class Bounce;

void spawnBox(shared_ptr<World> world, vec3 point)
{
    shared_ptr<Entity> box = world->addEntity();

    shared_ptr<MeshRenderer> m = box->addComponent<MeshRenderer>();
    m->mesh = 7;
    m->material = 9;
    shared_ptr<Transform> meshTransform = box->addComponent<Transform>();
    m->transform = meshTransform;
    meshTransform->setGlobalTransform(TransformData(point, quat(0,0,0,1), vec3(1, 1, 1)));
    shared_ptr<BoxCollider> collider = box->addComponent<BoxCollider>();
    collider->setExtents(vec3(1, 1, 1) * 0.5f);
    collider->transform = meshTransform;
    shared_ptr<RigidBody> body = box->addComponent<RigidBody>();
    body->transform = meshTransform;
    body->mass = 10;
    body->colliders.push_back(collider);
    box->addComponent<ApplyGravity>();
    box->addComponent<Bounce>();
}

class ControlledEntity : public Component
{
public:
    ControlledEntity() : Component(get_id(ControlledEntity)) {}
};

class CameraLookSystem : public System
{
public:
    weak_ptr<InputSystem> IS;
    bool* running = nullptr;
    float time = 0.f;

    virtual void frameTick(float delta) override {
        time += delta;
        shared_ptr<InputSystem> ISptr = IS.lock();
        if(ISptr) {
            if(ISptr->isActionDown(0, "escape", false)) {
                *running = false;
                ISptr->setTargetWindow(nullptr);
            }
        }
        Query<shared_ptr<Transform>> c = getWorld()->queryComponents(get_id(ControlledEntity))
            .map_ptr<Camera>(mapToSibling<Camera>)
            .map_ptr<Transform>(mapToTransform);
        for(shared_ptr<Transform> transform : c) {
            if(!transform) {
                continue;
            }
            TransformData td = transform->getRelativeTransform();
            vec3 eulerRot = toAxisRotator(td.rotation);
            eulerRot.x -= ISptr->getActionValue(0, "lookYaw", false) * 0.1f;
            eulerRot.y = clamp(eulerRot.y - ISptr->getActionValue(0, "lookPitch", false) * 0.1f, -89.f, 89.f);
            td.rotation = fromAxisRotator(eulerRot);
            transform->setRelativeTransform(td);
        }
    }
    virtual void gameplayTick(float delta) override {
        shared_ptr<InputSystem> ISptr = IS.lock();
        Query<shared_ptr<Transform>> c = getWorld()->queryComponents(get_id(ControlledEntity))
            .map_ptr<Camera>(mapToSibling<Camera>)
            .map_ptr<Transform>(mapToTransform);
        for(shared_ptr<Transform> transform : c) {
            if(!transform) {
                continue;
            }
            TransformData td = transform->getRelativeTransform();
            
            td.translation += (td.rotation * vec3(0,0,-1)) * ISptr->getActionValue(0, "forward", true) * delta * 5.f;
            td.translation += (td.rotation * vec3(-1,0,0)) * ISptr->getActionValue(0, "left", true) * delta * 5.f;
            td.translation += (td.rotation * vec3(0,1,0)) * ISptr->getActionValue(0, "up", true) * delta * 5.f;
            transform->setRelativeTransform(td);
        }
    }
};

class BoxSpawner : public System
{
public:
    weak_ptr<InputSystem> IS;
    weak_ptr<PhysicsSystem> PS;
    bool lmb_down = false;
    bool rmb_down = false;

    virtual void gameplayTick(float delta) override {
        shared_ptr<InputSystem> ISptr = IS.lock();
        shared_ptr<PhysicsSystem> PSptr = PS.lock();
        if(ISptr->isActionDown(0, "lmb", true)) {
            // Workaround until we get isActionPressed working properly.
            if(lmb_down) {
                return;
            }
            lmb_down = true;
            Query<shared_ptr<Transform>> t = getWorld()->queryComponents(get_id(ControlledEntity))
                .map_ptr<Camera>(mapToSibling<Camera>)
                .map_ptr<Transform>(mapToTransform);
            for(shared_ptr<Transform> transform : t) {
                TransformData td = transform->getGlobalTransform();
                spawnBox(getWorld(), td.transformPoint(vec3(0, 0, -5)));
            }
        }
        else {
            lmb_down = false;
        }
        if(ISptr->isActionDown(0, "rmb", true)) {
            // Workaround until we get isActionPressed working properly.
            if(rmb_down) {
                return;
            }
            rmb_down = true;
            Query<shared_ptr<Transform>> t = getWorld()->queryComponents(get_id(ControlledEntity))
                .map_ptr<Camera>(mapToSibling<Camera>)
                .map_ptr<Transform>(mapToTransform);
            for(shared_ptr<Transform> transform : t) {
                TransformData td = transform->getGlobalTransform();
                RaycastHit hit = PSptr->rayCast(td.translation, td.forward(), 10000, transform->getOwner());
                if(hit) {
                    shared_ptr<CollisionObject> obj = hit.obj.lock();
                    if(obj->getTypeId() == get_id(RigidBody)) {
                        getWorld()->removeEntity(obj->getOwner());
                    }
                }
            }
        }
        else {
            rmb_down = false;
        }
    }
};

class Bounce : public Component
{
public:
    Bounce() : Component(get_id(Bounce)) {}
};

class BoxBouncer : public System
{
public:
    float impulse = 2.5f;

    virtual void gameplayTick(float delta) override {
        auto query = getWorld()->queryComponents(get_id(Bounce))
            .map_ptr<RigidBody>(mapToSibling<RigidBody>);
        for(shared_ptr<RigidBody> body : query) {
            for(CollisionObject::Hit& hit : body->getHits()) {
                if(hit.other.lock()->getTypeId() == get_id(Trigger)) {
                    continue;
                }
                for(CollisionObject::Contact& contact : hit.contacts) {
                    body->addPointImpulse(impulse * contact.normal, contact.worldPoint);
                }
            }
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
        loader.addAssetType(typeid(Font), Font::build);
    }
    shared_ptr<Mesh> box = Mesh::makeBox(vec3(0.5f, 0.5f, 0.5f));
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
            materialData->setVec3Property("albedo", vec3(1,1,1));
            loader.addAssetData(9, typeid(Material), materialData);
        }
        loader.addAssetData(10, typeid(Texture), Texture::createAssetData("font.tpk"));
        loader.addAssetData(11, typeid(Font), Font::createAssetData(12, "font.fnt"));
        {
            auto textureData = RenderableTexture::createAssetData(10);
            textureData->wrapU = RenderableTexture::Clamp;
            textureData->wrapV = RenderableTexture::Clamp;
            textureData->minFilter = RenderableTexture::Linear;
            textureData->magFilter = RenderableTexture::Linear;
            textureData->minFilterMipMap = RenderableTexture::Linear;
            textureData->mipMapLevels = 1;
            loader.addAssetData(12, typeid(RenderableTexture), textureData);
        }
    }

    Universe U;
    U.gameplayRate = 60;
    bool running = true;

    shared_ptr<World> w = U.addWorld();
    shared_ptr<RenderSystem> RS = w->addSystem<RenderSystem>(-10000);
    RS->targetSurface = &window;
    RS->swapBuffers = false;
    shared_ptr<InputSystem> IS = w->addSystem<InputSystem>(20);
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
    //IS->setCursor(true, true);

    //shared_ptr<CameraLookSystem> CLS = w->addSystem<CameraLookSystem>(10);
    //CLS->IS = IS;
    //CLS->running = &running;

    w->addSystem<GravitySystem>(5);

    shared_ptr<PhysicsSystem> Physics = w->addSystem<PhysicsSystem>(0);
    Physics->setGravity(vec3(0,0,0));

    //w->addSystem<BoxBouncer>(-5);

    //shared_ptr<BoxSpawner> spawner = w->addSystem<BoxSpawner>(6);
    //spawner->IS = IS;
    //spawner->PS = Physics;

    shared_ptr<UIElement> element;
    shared_ptr<UIElement> defaultFocus;
    {
        shared_ptr<Container> layout = make_shared<Container>();
        layout->layoutAlgorithm = new OverlayLayout();
        element = layout;

        shared_ptr<Box> box = make_shared<Box>();
        box->layoutAlgorithm = new ListLayout<ListDirection::Row>(30);
        box->anchors = vec4(0, 1, 1, 1);
        box->origin.y = 1;
        box->position.y = -15;
        box->margin.x = 15;
        box->margin.z = 15;
        box->size.y = 150;
        box->cornerRadii = vec4(10, 10, 0, 0);
        box->colour = vec4(1, 0, 0, 1);
        box->padding = vec4(1,1,1,1) * 30.f;
        layout->addChild(box);

        shared_ptr<Button> btn1 = make_shared<Button>();
        btn1->layoutAlgorithm = new OverlayLayout();
        btn1->defaultColour = vec4(0.75f, 0.75f, 0.75f, 1.0f);
        btn1->hoveredColour = vec4(0.9f, 0.9f, 0.9f, 1.0f);
        btn1->pressedColour = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        btn1->padding = vec4(1,1,1,1) * 15.f;
        btn1->cornerRadii = vec4(10, 10, 10, 10);
        btn1->size.x = 300;
        box->addChild(btn1);

        shared_ptr<Button> btn2 = make_shared<Button>();
        btn2->layoutAlgorithm = new OverlayLayout();
        btn2->defaultColour = vec4(0.75f, 0.75f, 0.75f, 1.0f);
        btn2->hoveredColour = vec4(0.9f, 0.9f, 0.9f, 1.0f);
        btn2->pressedColour = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        btn2->padding = vec4(1,1,1,1) * 15.f;
        btn2->cornerRadii = vec4(10, 10, 10, 10);
        btn2->weight = 1;
        box->addChild(btn2);

        shared_ptr<Button> btn3 = make_shared<Button>();
        btn3->layoutAlgorithm = new OverlayLayout();
        btn3->defaultColour = vec4(0.75f, 0.75f, 0.75f, 1.0f);
        btn3->hoveredColour = vec4(0.9f, 0.9f, 0.9f, 1.0f);
        btn3->pressedColour = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        btn3->padding = vec4(1,1,1,1) * 15.f;
        btn3->cornerRadii = vec4(10, 10, 10, 10);
        box->addChild(btn3);

        shared_ptr<Text> text = make_shared<Text>();
        text->setFontSize(20.0f);
        text->setLineSpacing(1.0f);
        text->font = 11;
        text->colour = vec4(0,0,0,1);
        text->setText("Push Me!\nHello world It's a me, Mario!");
        text->setTextAlignment(Font::Center);
        btn3->addChild(text);

        btn1->neighbours[UIElement::Right] = btn2;
        btn2->neighbours[UIElement::Right] = btn3;
        
        btn3->neighbours[UIElement::Left] = btn2;
        btn2->neighbours[UIElement::Left] = btn1;

        defaultFocus = btn1;
    }

    shared_ptr<UISystem> ui = w->addSystem<UISystem>(-11000);
    ui->addElement(element);
    //ui->uiScale = 1.5f;
    ui->targetSurface = &window;
    ui->setDefaultFocus(defaultFocus);

    w->addEntity();

    vec3 floorPos(0,0,0);
    vec3 camPos(0,5,5);
    vec3 boxPos(0,5,0);
    vec3 gravPos(0, 3, -7.5);
    vec3 gravSize(100,100,100);//(15, 5, 7.5);

    shared_ptr<Entity> floor = w->addEntity();
    {
        shared_ptr<MeshRenderer> m = floor->addComponent<MeshRenderer>();
        m->mesh = 7;
        m->material = 9;
        shared_ptr<Transform> meshTransform = floor->addComponent<Transform>();
        m->transform = meshTransform;
        meshTransform->setGlobalTransform(TransformData(floorPos, quat(0,0,0,1), vec3(15, 1, 15)));
        shared_ptr<BoxCollider> collider = floor->addComponent<BoxCollider>();
        collider->setExtents(vec3(1, 1, 1) * 0.5f);
        collider->transform = meshTransform;
        shared_ptr<StaticBody> body = floor->addComponent<StaticBody>();
        body->transform = meshTransform;
        body->colliders.push_back(collider);
    }

    shared_ptr<Entity> gravRegion = w->addEntity();
    {
        shared_ptr<Transform> t = gravRegion->addComponent<Transform>();
        t->setGlobalTransform(TransformData(gravPos, quat(0,0,0,1), gravSize));
        shared_ptr<BoxCollider> collider = gravRegion->addComponent<BoxCollider>();
        collider->setExtents(vec3(1,1,1) * 0.5f);
        collider->transform = t;
        shared_ptr<Trigger> trig = gravRegion->addComponent<Trigger>();
        trig->transform = t;
        trig->colliders.push_back(collider);
        gravRegion->addComponent<GravityRegion>();
    }
    
    for(int i = 0; i < 30; i++) {
        vec3 point(
            rand() * 1.f / RAND_MAX * 10 - 5.f,
            rand() * 1.f / RAND_MAX * 5,
            rand() * 1.f / RAND_MAX * 10 - 5.f);
        spawnBox(w, point);
    }
    
    shared_ptr<Entity> camera = w->addEntity();
    {
        shared_ptr<Transform> camTransform = camera->addComponent<Transform>();
        shared_ptr<Camera> cam = camera->addComponent<Camera>();
        cam->transform = camTransform;
        camTransform->setRelativeTransform(TransformData(camPos));
        shared_ptr<SphereCollider> collider = camera->addComponent<SphereCollider>();
        collider->transform = camTransform;
        collider->setRadius(0.5f);
        shared_ptr<KinematicBody> body = camera->addComponent<KinematicBody>();
        body->transform = camTransform;
        body->colliders.push_back(collider);
        camera->addComponent<ControlledEntity>();
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
            cout << "FPS: " << (1.0f / delta) << endl;
        }

        ui->clearInputStates();
        ui->onMouseMove(IS->getMousePosition());
        if(IS->isMousePressed(0)) { ui->onMousePressed(UISystem::MouseButton::LMB); }
        if(IS->isMouseReleased(0)) { ui->onMouseReleased(UISystem::MouseButton::LMB); }
        if(IS->isMousePressed(1)) { ui->onMousePressed(UISystem::MouseButton::RMB); }
        if(IS->isMouseReleased(1)) { ui->onMouseReleased(UISystem::MouseButton::RMB); }
        if(IS->isKeyPressed(GLFW_KEY_ENTER)) { ui->onAcceptPressed(); }
        if(IS->isKeyReleased(GLFW_KEY_ENTER)) { ui->onAcceptReleased(); }
        if(IS->isKeyPressed(GLFW_KEY_W)) { ui->changeFocus(UIElement::Up); }
        if(IS->isKeyPressed(GLFW_KEY_A)) { ui->changeFocus(UIElement::Left); }
        if(IS->isKeyPressed(GLFW_KEY_S)) { ui->changeFocus(UIElement::Down); }
        if(IS->isKeyPressed(GLFW_KEY_D)) { ui->changeFocus(UIElement::Right); }
        if(IS->isKeyPressed(GLFW_KEY_UP)) { ui->changeFocus(UIElement::Up); }
        if(IS->isKeyPressed(GLFW_KEY_LEFT)) { ui->changeFocus(UIElement::Left); }
        if(IS->isKeyPressed(GLFW_KEY_DOWN)) { ui->changeFocus(UIElement::Down); }
        if(IS->isKeyPressed(GLFW_KEY_RIGHT)) { ui->changeFocus(UIElement::Right); }

        U.tick(delta);
    } while(running && !window.wantsClose());

    return 0;
}
