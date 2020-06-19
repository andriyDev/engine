
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

vec2 Container::layout(hash_map<const UIElement*, vec2>& desiredSizes)
{
    vector<vec2> sizes;
    sizes.reserve(elements.size());
    for(const auto& child : elements) {
        vec2 size = child->layout(desiredSizes);
        sizes.push_back(size);
    }

    vec2 desiredSize(0,0);
    if(layoutAlgorithm) {
        desiredSize = layoutAlgorithm->computeDesiredSize(this, sizes, elements);
    }
    desiredSize += vec2(padding.x + padding.z, padding.y + padding.w);
    desiredSizes.insert(make_pair(this, desiredSize));
    return desiredSize;
}

void Container::render(vec4 rect, vec4 mask, vec2 surfaceSize, const hash_map<const UIElement*, vec2>& desiredSizes)
{
    renderSelf(rect, mask, surfaceSize);
    vector<vec4> boxes;
    if(layoutAlgorithm) {
        boxes = layoutAlgorithm->layoutElements(this, rect + padding * vec4(1, 1, -1, -1), elements, desiredSizes);
    }
    for(int i = 0; i < elements.size(); i++) {
        vec4& box = layoutAlgorithm ? boxes[i] : rect;
        elements[i]->render(box, maskChildren ? rect : mask, surfaceSize, desiredSizes);
    }
}
