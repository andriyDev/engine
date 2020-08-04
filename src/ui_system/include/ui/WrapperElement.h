
#pragma once

#include "std.h"
#include "ui/UIElement.h"

class WrapperElement : public UIElement
{
public:
    virtual void render(vec4 mask, vec2 surfaceSize) override;
    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) override;
    virtual shared_ptr<UIElement> queryLayout(vec2 point, vec4 mask, bool onlyInteractive) override;
protected:
    virtual pair<UILayoutRequest, bool> computeLayoutRequest() override;
    virtual hash_map<UIElement*, vec4> computeChildLayouts() override;
    
    bool maskChildren = true;
    bool useUnpaddedBoxAsMask = false;
};
