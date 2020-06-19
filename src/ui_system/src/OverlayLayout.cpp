
#include "ui/ContainerLayouts.h"

vector<vec4> OverlayLayout::layoutElements(const UIElement* element, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements,
    const hash_map<const UIElement*, vec2>& desiredSizes) const
{
    vector<vec4> layout;
    layout.reserve(elements.size());
    for(shared_ptr<UIElement> element : elements) {
        layout.push_back(element->adjustRect(rect, desiredSizes));
    }
    return layout;
}

vec2 OverlayLayout::computeDesiredSize(const UIElement* element, const vector<vec2>& childSizes,
    const vector<shared_ptr<UIElement>>& elements) const
{
    vec2 bounds = vec2(0,0);
    for(int i = 0; i < elements.size(); i++) {
        vec2 size = childSizes[i] + vec2(elements[i]->margin.x + elements[i]->margin.z,
            elements[i]->margin.y + elements[i]->margin.w);
        bounds = vec2(max(bounds.x, size.x), max(bounds.y, size.y));
    }
    return bounds;
}
