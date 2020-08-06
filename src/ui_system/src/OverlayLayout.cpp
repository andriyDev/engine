
#include "ui/ContainerLayouts.h"

vector<vec4> OverlayLayout::layoutElements(const UIElement* rootElement, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements) const
{
    vector<vec4> layout;
    layout.reserve(elements.size());
    for(shared_ptr<UIElement> element : elements) {
        layout.push_back(element->adjustRect(rect));
    }
    return layout;
}

UILayoutRequest OverlayLayout::computeLayoutRequest(const UIElement* rootElement,
    const vector<shared_ptr<UIElement>>& elements) const
{
    UILayoutRequest info;
    vec2& bounds = info.desiredSize;
    bounds = vec2(0,0);
    for(int i = 0; i < elements.size(); i++) {
        const UIElement::LayoutDetails& eld = elements[i]->getLayoutDetails();
        vec2 size = elements[i]->getLayoutRequest().desiredSize + vec2(
            abs(eld.margin.x) + abs(eld.margin.z),
            abs(eld.margin.y) + abs(eld.margin.w));
        bounds = vec2(glm::max(bounds.x, size.x), glm::max(bounds.y, size.y));
    }
    return info;
}
