
#pragma once

#include "std.h"

#include "ui/Container.h"
#include "renderer/Material.h"

class Box : public Container
{
public:
    Box();

    vec4 colour;

    virtual void renderSelf(vec4 rect, vec4 mask, vec2 surfaceSize) override;
protected:
    static shared_ptr<MaterialProgram> boxProgram;
    static shared_ptr<Material> boxMaterial;
};
