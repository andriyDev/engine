
#pragma once

#include "std.h"

class UIElement
{
public:
    virtual vec2 layout(hash_map<const UIElement*, vec2>& desiredSizes) const = 0;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, vec2>& desiredSizes) = 0;
};
