
#include "renderer/RenderSystem.h"

#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/Camera.h"

#include "ComponentTypes.h"

void RenderSystem::frameTick(float delta, float tickPercent)
{
    Query<Camera*> cameras = getWorld()->queryComponents()
        .filter(filterByTypeId(CAMERA_ID))
        .cast<Camera*>();
    Query<MeshRenderer*> meshes = getWorld()->queryComponents()
        .filter(filterByTypeId(MESH_RENDERER_ID))
        .cast<MeshRenderer*>();

    float screenAspect = 1280.f / 720.f; // TODO

    for(Camera* camera : cameras) {
        glm::mat4 vpMatrix = camera->getVPMatrix(tickPercent, screenAspect);

        for(MeshRenderer* renderer : meshes) {
            // Don't render a mesh with no material.
            if(!renderer->material) {
                continue;
            }
            renderer->mesh->bind();
            renderer->material->use();
            Transform* transform = Transform::getComponentTransform(renderer);
            glm::mat4 model = transform ? transform->getGlobalTransform(tickPercent).toMat4() : glm::mat4(1.0);
            renderer->material->setMVP(model, vpMatrix);
            renderer->mesh->render();
        }
    }
}

void RenderSystem::gameplayTick(float delta)
{
    Query<Transform*> transforms = getWorld()->queryComponents()
        .filter(filterByTypeId(TRANSFORM_ID))
        .cast<Transform*>();
    for(Transform* transform : transforms)
    {
        transform->previousTransform[0] = transform->previousTransform[1];
        transform->previousTransform[1] = transform->relativeTransform;
    }
}
