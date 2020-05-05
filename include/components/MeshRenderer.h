
#pragma once

#include "core/Component.h"
#include "renderer/Material.h"
#include "renderer/RenderableMesh.h"

#include "ComponentTypes.h"

class MeshRenderer : public Component
{
public:
    MeshRenderer()
        : Component(MESH_RENDERER_ID)
    { }

    std::shared_ptr<RenderableMesh> mesh = nullptr;
    std::shared_ptr<Material> material = nullptr;
};
