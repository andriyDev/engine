
#include "renderer/RenderSystem.h"

#include "core/World.h"
#include "core/Query.h"
#include "core/Entity.h"
#include "core/Component.h"

#include "components/Transform.h"
#include "renderer/MeshRenderer.h"
#include "renderer/Camera.h"

void RenderSystem::init()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glCullFace(GL_BACK);
}

void RenderSystem::frameTick(float delta)
{
    Query<shared_ptr<Camera>> cameras = getWorld()->queryComponents(get_id(Camera))
        .cast_ptr<Camera>();
    Query<shared_ptr<MeshRenderer>> meshes = getWorld()->queryComponents(get_id(MeshRenderer))
        .cast_ptr<MeshRenderer>();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec2 surfaceSize = targetSurface->getSize();
    float screenAspect = surfaceSize.y == 0 ? 1 : (surfaceSize.x / surfaceSize.y);

    for(shared_ptr<Camera> camera : cameras) {
        mat4 vpMatrix = camera->getVPMatrix(screenAspect);

        for(shared_ptr<MeshRenderer> renderer : meshes) {
            shared_ptr<RenderableMesh> mesh = renderer->mesh.resolve(Deferred);
            shared_ptr<Material> material = renderer->material.resolve(Deferred);
            // Don't render a mesh where the mesh or material are in a bad state.
            if(!mesh || !material) {
                continue;
            }
            mesh->bind();
            material->use();
            shared_ptr<Transform> transform = renderer->getTransform();
            mat4 model = transform ? transform->getGlobalTransform().toMat4() : mat4(1.0);
            material->setMVP(model, vpMatrix);
            mesh->render();
        }
    }
    if(swapBuffers) {
        targetSurface->swapBuffers();
    }
}
