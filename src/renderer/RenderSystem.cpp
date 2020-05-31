
#include "renderer/RenderSystem.h"

#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/Camera.h"

void RenderSystem::init()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glCullFace(GL_BACK);
}

void RenderSystem::frameTick(float delta)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Query<std::shared_ptr<Camera>> cameras = getWorld()->queryComponents()
        .filter(filterByType<Camera>)
        .cast_ptr<Camera>();
    Query<std::shared_ptr<MeshRenderer>> meshes = getWorld()->queryComponents()
        .filter(filterByType<MeshRenderer>)
        .cast_ptr<MeshRenderer>();

    float screenAspect = 1280.f / 720.f; // TODO

    for(std::shared_ptr<Camera> camera : cameras) {
        glm::mat4 vpMatrix = camera->getVPMatrix(screenAspect);

        for(std::shared_ptr<MeshRenderer> renderer : meshes) {
            std::shared_ptr<RenderableMesh> mesh = renderer->mesh.resolve(Deferred);
            std::shared_ptr<Material> material = renderer->material.resolve(Deferred);
            // Don't render a mesh where the mesh or material are in a bad state.
            if(!mesh || !material) {
                continue;
            }
            mesh->bind();
            material->use();
            std::shared_ptr<Transform> transform = renderer->getTransform();
            glm::mat4 model = transform ? transform->getGlobalTransform().toMat4() : glm::mat4(1.0);
            material->setMVP(model, vpMatrix);
            mesh->render();
        }
    }
    targetWindow->swapBuffers();
}
