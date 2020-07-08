
#include "ui/WrapperElement.h"

void WrapperElement::render(vec4 mask, vec2 surfaceSize)
{
    if(wrappedElement) {
        wrappedElement->render(mask, surfaceSize);
    }
}

void WrapperElement::update(float delta, shared_ptr<UISystem> ui)
{
    if(wrappedElement) {
        wrappedElement->update(delta, ui);
    }
}

shared_ptr<UIElement> WrapperElement::queryLayout(vec2 point, vec4 mask, bool onlyInteractive)
{
    if(wrappedElement) {
        shared_ptr<UIElement> response = wrappedElement->queryLayout(point, mask, onlyInteractive);
        if(response) {
            return response;
        }
    }
    return UIElement::queryLayout(point, mask, onlyInteractive);
}

pair<UILayoutRequest, bool> WrapperElement::computeLayoutRequest()
{
    UILayoutRequest info = wrappedElement ? wrappedElement->getLayoutRequest() : UILayoutRequest({vec2(0,0), false});
    info.desiredSize += vec2(padding.x + padding.z, padding.y + padding.w);
    return make_pair(info, false);
}

hash_map<UIElement*, vec4> WrapperElement::computeChildLayouts()
{
    vec4 box = getLayoutBox() + padding * vec4(1,1,-1,-1);
    hash_map<UIElement*, vec4> layout;
    if(wrappedElement) {
        layout.insert(make_pair(wrappedElement.get(), box));
    }
    return layout;
}

bool WrapperElement::updateChildLayoutRequests()
{
    if(!wrappedElement) {
        return false;
    }
    return wrappedElement->updateLayoutRequest();
}

void WrapperElement::releaseChild(shared_ptr<UIElement> element)
{
    if(element == wrappedElement) {
        wrappedElement = nullptr;
    }
}
