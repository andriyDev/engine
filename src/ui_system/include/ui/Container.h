
#pragma once

#include "std.h"

#include "UIElement.h"
#include "renderer/Material.h"
#include "ContainerLayouts.h"

class Container : public UIElement
{
public:
    virtual ~Container();

    bool maskChildren = true;
    ContainerLayout* layoutAlgorithm = nullptr;

    virtual void addChild(shared_ptr<UIElement> element);
    virtual void removeChild(shared_ptr<UIElement> element);
    virtual void clearChildren();

    virtual vec2 layout(hash_map<const UIElement*, vec2>& desiredSizes) override;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, vec2>& desiredSizes) override;
    
    virtual void renderSelf(vec4 rect, vec4 mask, vec2 surfaceSize) {};
protected:
    vector<shared_ptr<UIElement>> elements;
};
