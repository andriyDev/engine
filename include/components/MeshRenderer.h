
#pragma once

#include "components/Transform.h"
#include "renderer/Material.h"
#include "renderer/RenderableMesh.h"

#include "ComponentTypes.h"

class MeshRenderer : public Transformable
{
public:
    MeshRenderer() : Transformable(MESH_RENDERER_ID) { }

    std::shared_ptr<RenderableMesh> mesh = nullptr;
    std::shared_ptr<Material> material = nullptr;
};
