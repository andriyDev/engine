
#pragma once

#include "std.h"

#include "core/System.h"
#include "renderer/RenderSurface.h"

#include "UIElement.h"

class UISystem : public System
{
    virtual void init() override;
    virtual void frameTick(float delta) override;
public:
    RenderSurface* targetSurface = nullptr;
    bool swapBuffers = true;
    float uiScale = 1.0f;

    void addElement(shared_ptr<UIElement> element);
    void removeElement(shared_ptr<UIElement> element);
private:
    hash_set<shared_ptr<UIElement>> elements;
    vec2 lastSurfaceSize = vec2(0,0);
};
