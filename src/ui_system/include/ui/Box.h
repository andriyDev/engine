
#pragma once

#include "std.h"

#include "UIElement.h"
#include "renderer/Material.h"

class Box : public UIElement
{
public:
    Box();

    vec4 colour;

    virtual vec2 layout(hash_map<const UIElement*, vec2>& desiredSizes) const override;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, vec2>& desiredSizes) override;
protected:
    static shared_ptr<MaterialProgram> boxProgram;
    static shared_ptr<Material> boxMaterial;
};
