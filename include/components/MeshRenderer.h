
#pragma once

#include "core/Component.h"
#include "renderer/RenderableMesh.h"

#include "ComponentTypes.h"

class MeshRenderer : public Component
{
public:
    MeshRenderer()
        : Component(MESH_RENDERER_ID)
    { }

    RenderableMesh* mesh = nullptr;
};
