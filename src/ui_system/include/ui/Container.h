
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
    bool useUnpaddedBoxAsMask = false;
    ContainerLayout* layoutAlgorithm = nullptr;

    inline const vector<shared_ptr<UIElement>>& getChildren() const { return getElementChildren(); }
    virtual void addChild(shared_ptr<UIElement> element);
    virtual void removeChild(shared_ptr<UIElement> element);
    virtual void clearChildren();

    virtual void render(vec4 mask, vec2 surfaceSize) override;

    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) override;

    virtual shared_ptr<UIElement> queryLayout(vec2 point, vec4 mask, bool onlyInteractive) override;
protected:
    virtual pair<UILayoutRequest, bool> computeLayoutRequest() override;
    virtual hash_map<UIElement*, vec4> computeChildLayouts() override;
    
    virtual void renderSelf(vec4 mask, vec2 surfaceSize) {}
};
