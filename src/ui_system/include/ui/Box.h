
#pragma once

#include "std.h"

#include "ui/Container.h"
#include "renderer/Material.h"

class Box : public Container
{
public:
    Box();

    vec4 colour = vec4(1,1,1,1);
    // The radius of each corner in order of TL TR BL BR
    vec4 cornerRadii = vec4(0,0,0,0);

protected:
    virtual void renderSelf(vec4 mask, vec2 surfaceSize) override;

    static shared_ptr<MaterialProgram> boxProgram;
    static shared_ptr<Material> boxMaterial;
};
