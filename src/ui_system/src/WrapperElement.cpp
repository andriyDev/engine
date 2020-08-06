
#include "ui/WrapperElement.h"

void WrapperElement::render(vec4 mask, vec2 surfaceSize)
{
    if(maskChildren) {
        mask = intersect_boxes(mask, useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox());
    }
    for(const shared_ptr<UIElement>& element : getElementChildren()) {
        element->render(mask, surfaceSize);
    }
}

void WrapperElement::update(float delta, vec4 mask, shared_ptr<UISystem> ui)
{
    if(maskChildren) {
        mask = intersect_boxes(mask, useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox());
    }
    for(const shared_ptr<UIElement>& element : getElementChildren()) {
        element->update(delta, mask, ui);
    }
}

shared_ptr<UIElement> WrapperElement::queryLayout(vec2 point, vec4 mask, bool onlyInteractive)
{
    vec4 childMask = maskChildren ? intersect_boxes(mask,
        useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox()) : mask;
    if(isPointInBox(childMask, point)) {
        for(auto it = getElementChildren().rbegin(); it != getElementChildren().rend(); ++it) {
            const shared_ptr<UIElement>& element = *it;
            shared_ptr<UIElement> response = element->queryLayout(point, childMask, onlyInteractive);
            if(response) {
                return response;
            }
        }
    }
    return UIElement::queryLayout(point, mask, onlyInteractive);
}

pair<UILayoutRequest, bool> WrapperElement::computeLayoutRequest()
{
    UILayoutRequest info;
    info.desiredSize = vec2(0,0);
    info.maintainAspect = false;
    for(const shared_ptr<UIElement>& element : getElementChildren()) {
        info.desiredSize = glm::max(info.desiredSize, element->getLayoutRequest().desiredSize);
    }
    const vec4& padding = getLayoutDetails().padding;
    info.desiredSize += vec2(padding.x + padding.z, padding.y + padding.w);
    return make_pair(info, false);
}

hash_map<UIElement*, vec4> WrapperElement::computeChildLayouts()
{
    hash_map<UIElement*, vec4> result;
    vec4 paddedBox = getPaddedLayoutBox();
    for(const shared_ptr<UIElement>& element : getElementChildren()) {
        result.insert(make_pair(element.get(), paddedBox));
    }
    return result;
}
