
#include "ui/Container.h"
#include "resources/Shader.h"

Container::~Container()
{
    delete layoutAlgorithm;
}

void Container::addChild(shared_ptr<UIElement> element)
{
    addElementChild(element);
}

void Container::removeChild(shared_ptr<UIElement> element)
{
    removeElementChild(element);
}

void Container::clearChildren()
{
    clearElementChildren();
}

pair<UILayoutRequest, bool> Container::computeLayoutRequest()
{
    UILayoutRequest info;
    if(layoutAlgorithm) {
        info = layoutAlgorithm->computeLayoutRequest(this, getChildren());
    } else {
        info.desiredSize = vec2(0,0);
        info.maintainAspect = false;
        for(const shared_ptr<UIElement>& element : getChildren()) {
            info.desiredSize = glm::max(info.desiredSize, element->getLayoutRequest().desiredSize);
        }
    }
    info.desiredSize += vec2(layoutDetails.padding.x + layoutDetails.padding.z,
        layoutDetails.padding.y + layoutDetails.padding.w);
    return make_pair(info, false);
}

hash_map<UIElement*, vec4> Container::computeChildLayouts()
{
    hash_map<UIElement*, vec4> result;
    vector<vec4> boxes;
    vec4 paddedBox = getLayoutBox() + layoutDetails.padding * vec4(1, 1, -1, -1);
    
    const vector<shared_ptr<UIElement>>& children = getChildren();
    if(layoutAlgorithm) {
        boxes = layoutAlgorithm->layoutElements(this, paddedBox, children);
    }
    for(uint i = 0; i < children.size(); i++) {
        vec4& box = layoutAlgorithm ? boxes[i] : paddedBox;
        result.insert(make_pair(children[i].get(), box));
    }
    return result;
}

void Container::render(vec4 mask, vec2 surfaceSize)
{
    renderSelf(mask, surfaceSize);
    if(maskChildren) {
        mask = intersect_boxes(mask, useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox());
    }
    for(const shared_ptr<UIElement>& element : getChildren()) {
        element->render(mask, surfaceSize);
    }
}

void Container::update(float delta, vec4 mask, shared_ptr<UISystem> ui)
{
    if(maskChildren) {
        mask = intersect_boxes(mask, useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox());
    }
    for(const shared_ptr<UIElement>& element : getChildren()) {
        element->update(delta, mask, ui);
    }
}

shared_ptr<UIElement> Container::queryLayout(vec2 point, vec4 mask, bool onlyInteractive)
{
    vec4 childMask = maskChildren ? intersect_boxes(mask, getLayoutBox()) : mask;
    if(isPointInBox(childMask, point)) {
        for(auto it = getChildren().rbegin(); it != getChildren().rend(); ++it) {
            const shared_ptr<UIElement>& element = *it;
            shared_ptr<UIElement> response = element->queryLayout(point, childMask, onlyInteractive);
            if(response) {
                return response;
            }
        }
    }
    return UIElement::queryLayout(point, mask, onlyInteractive);
}
