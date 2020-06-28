
#include "ui/Container.h"
#include "resources/Shader.h"

#include "UIUtil.h"

Container::~Container()
{
    delete layoutAlgorithm;
}

void Container::addChild(shared_ptr<UIElement> element)
{
    elements.push_back(element);
}

void Container::removeChild(shared_ptr<UIElement> element)
{
    auto it = find(elements.begin(), elements.end(), element);
    if(it != elements.end()) {
        elements.erase(it);
    }
}

void Container::clearChildren()
{
    elements.clear();
}

UILayoutInfo Container::layout(hash_map<const UIElement*, UILayoutInfo>& layoutInfo)
{
    UILayoutInfo info;
    vector<UILayoutInfo> childInfo;
    childInfo.reserve(elements.size());
    for(const auto& child : elements) {
        UILayoutInfo cinfo = child->layout(layoutInfo);
        childInfo.push_back(cinfo);
    }

    if(layoutAlgorithm) {
        info = layoutAlgorithm->computeLayoutInfo(this, childInfo, elements);
    }
    info.desiredSize += vec2(padding.x + padding.z, padding.y + padding.w);
    layoutInfo.insert(make_pair(this, info));
    return info;
}

void Container::render(vec4 rect, vec4 mask, vec2 surfaceSize,
    const hash_map<const UIElement*, UILayoutInfo>& layoutInfo)
{
    renderSelf(rect, mask, surfaceSize);
    vector<vec4> boxes;
    if(layoutAlgorithm) {
        boxes = layoutAlgorithm->layoutElements(this, rect + padding * vec4(1, 1, -1, -1), elements, layoutInfo);
    }
    for(int i = 0; i < elements.size(); i++) {
        vec4& box = layoutAlgorithm ? boxes[i] : rect;
        elements[i]->render(box, maskChildren ? intersect_boxes(mask, rect) : mask, surfaceSize, layoutInfo);
    }
}
