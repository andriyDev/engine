
#include "renderer/RenderSystem.h"

#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/Camera.h"

#include "ComponentTypes.h"

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
        .filter(filterByTypeId(CAMERA_ID))
        .cast_ptr<Camera>();
    Query<std::shared_ptr<MeshRenderer>> meshes = getWorld()->queryComponents()
        .filter(filterByTypeId(MESH_RENDERER_ID))
        .cast_ptr<MeshRenderer>();

    float screenAspect = 1280.f / 720.f; // TODO

    for(std::shared_ptr<Camera> camera : cameras) {
        glm::mat4 vpMatrix = camera->getVPMatrix(screenAspect);

        for(std::shared_ptr<MeshRenderer> renderer : meshes) {
            // Don't render a mesh where the mesh or material are in a bad state.
            if(!renderer->mesh || !renderer->material
                || renderer->mesh->state != Resource::Success || !renderer->material->isUsable()) {
                continue;
            }
            renderer->mesh->bind();
            renderer->material->use();
            std::shared_ptr<Transform> transform = renderer->getTransform();
            glm::mat4 model = transform ? transform->getGlobalTransform().toMat4() : glm::mat4(1.0);
            renderer->material->setMVP(model, vpMatrix);
            renderer->mesh->render();
        }
    }
    targetWindow->swapBuffers();
}
