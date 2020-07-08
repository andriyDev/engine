
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
        vec2 size = elements[i]->getLayoutRequest().desiredSize + vec2(
            abs(elements[i]->margin.x) + abs(elements[i]->margin.z),
            abs(elements[i]->margin.y) + abs(elements[i]->margin.w));
        bounds = vec2(max(bounds.x, size.x), max(bounds.y, size.y));
    }
    return info;
}
