
#pragma once

#include "std.h"

class UIElement
{
public:
    enum Gravity {
        Start, End, Center, Fill
    };
    // We package these together to save memory since both options cannot be used at the same time.
    union {
        struct {
            // The point to expand from relative to itself (0 means left/top, 1 means right/bottom).
            vec2 origin;
            // The position of the origin if the element is free.
            vec2 position;
        };
        // The space between an element and its parent (going left, top, right, bottom).
        vec4 margin = vec4(0,0,0,0);
    };
    // If we are free to choose our size, what should it be? 0 means use the desiredSize.
    vec2 size = vec2(0,0);
    // The points to anchor this element relative to the space it occupies (0 meaning left/top, 1 meaning right/bottom).
    vec4 anchors = vec4(0,0,1,1);
    // The space between an element and its children (going left, top, right, bottom).
    vec4 padding = vec4(0,0,0,0);
    // If the element is free in the horizontal direction, how should the element prefer to be positioned.
    Gravity horizontalGravity = Fill;
    // If the element is free in the vertical direction, how should the element prefer to be positioned.
    Gravity verticalGravity = Fill;
    // If space (or lack of space) must be distributed between siblings, how much should be given to this element.
    float weight = 0;

    vec4 adjustRect(vec4 rect, const hash_map<const UIElement*, vec2>& desiredSizes) const;

    virtual vec2 layout(hash_map<const UIElement*, vec2>& desiredSizes) = 0;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, vec2>& desiredSizes) = 0;
};
