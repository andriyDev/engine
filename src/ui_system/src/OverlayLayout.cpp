
#include "ui/ContainerLayouts.h"

vector<vec4> OverlayLayout::layoutElements(const UIElement* element, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements,
    const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const
{
    vector<vec4> layout;
    layout.reserve(elements.size());
    for(shared_ptr<UIElement> element : elements) {
        layout.push_back(element->adjustRect(rect, layoutInfo));
    }
    return layout;
}

UILayoutInfo OverlayLayout::computeLayoutInfo(const UIElement* element, const vector<UILayoutInfo>& childLayout,
    const vector<shared_ptr<UIElement>>& elements) const
{
    UILayoutInfo info;
    vec2& bounds = info.desiredSize;
    bounds = vec2(0,0);
    for(int i = 0; i < elements.size(); i++) {
        vec2 size = childLayout[i].desiredSize + vec2(elements[i]->margin.x + elements[i]->margin.z,
            elements[i]->margin.y + elements[i]->margin.w);
        bounds = vec2(max(bounds.x, size.x), max(bounds.y, size.y));
    }
    return info;
}
