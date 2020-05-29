
#pragma once

#include "components/Transform.h"
#include "renderer/Material.h"
#include "renderer/RenderableMesh.h"

#include "ComponentTypes.h"

class MeshRenderer : public Transformable
{
public:
    MeshRenderer() : Transformable(MESH_RENDERER_ID) { }

    ResourceRef<RenderableMesh> mesh;
    ResourceRef<Material> material;
};
