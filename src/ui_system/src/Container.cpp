
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
    element->setParent(shared_from_this());
}

void Container::removeChild(shared_ptr<UIElement> element)
{
    auto it = find(elements.begin(), elements.end(), element);
    if(it != elements.end()) {
        elements.erase(it);
        element->setParent(nullptr);
    }
}

void Container::clearChildren()
{
    for(shared_ptr<UIElement> element : elements) {
        element->setParent(nullptr);
    }
    elements.clear();
}

void Container::releaseChild(shared_ptr<UIElement> element)
{
    auto it = find(elements.begin(), elements.end(), element);
    if(it != elements.end()) {
        elements.erase(it);
    }
}

pair<UILayoutRequest, bool> Container::computeLayoutRequest()
{
    UILayoutRequest info;
    if(layoutAlgorithm) {
        info = layoutAlgorithm->computeLayoutRequest(this, elements);
    }
    info.desiredSize += vec2(padding.x + padding.z, padding.y + padding.w);
    return make_pair(info, false);
}

hash_map<UIElement*, vec4> Container::computeChildLayouts()
{
    hash_map<UIElement*, vec4> result;
    vector<vec4> boxes;
    vec4 paddedBox = getLayoutBox() + padding * vec4(1, 1, -1, -1);
    
    if(layoutAlgorithm) {
        boxes = layoutAlgorithm->layoutElements(this, paddedBox, elements);
    }
    for(uint i = 0; i < elements.size(); i++) {
        vec4& box = layoutAlgorithm ? boxes[i] : paddedBox;
        result.insert(make_pair(elements[i].get(), box));
    }
    return result;
}

bool Container::updateChildLayoutRequests()
{
    bool stillDirty = false;
    for(shared_ptr<UIElement>& element : elements) {
        stillDirty = stillDirty || element->updateLayoutRequest();
    }
    return stillDirty;
}

void Container::render(vec4 mask, vec2 surfaceSize)
{
    renderSelf(mask, surfaceSize);
    for(shared_ptr<UIElement>& element : elements) {
        element->render(mask, surfaceSize);
    }
}