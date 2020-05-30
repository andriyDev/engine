
#pragma once

#include "components/Transform.h"
#include "renderer/Material.h"
#include "renderer/RenderableMesh.h"

class MeshRenderer : public Transformable
{
public:
    MeshRenderer() : Transformable(get_id(MeshRenderer)) { }

    ResourceRef<RenderableMesh> mesh;
    ResourceRef<Material> material;
};
