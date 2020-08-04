
#include "ui/UIElement.h"

vec4 UIElement::getPaddedLayoutBox() const
{
    return getLayoutBox() + layoutDetails.padding * vec4(1,1,-1,-1);
}

void adjustX(vec2& min, vec2& max, const UIElement::LayoutDetails& ld, const UILayoutRequest& info, bool canUseAspect)
{
    if(ld.anchors.x == ld.anchors.z) {
        float desiredWidth;
        if(ld.size.x == 0) {
            desiredWidth = canUseAspect && info.maintainAspect && info.desiredSize.y != 0
                ? info.desiredSize.x / info.desiredSize.y * (max.y - min.y)
                : info.desiredSize.x;
        } else {
            desiredWidth = ld.size.x;
        }
        
        min.x -= desiredWidth * ld.origin.x;
        max.x += desiredWidth * (1.f - ld.origin.x);

        min.x += ld.position.x;
        max.x += ld.position.x;
    } else {
        min.x += ld.margin.x;
        max.x -= ld.margin.z;
    }
}

void adjustY(vec2& min, vec2& max, const UIElement::LayoutDetails& ld, const UILayoutRequest& info, bool canUseAspect)
{
    if(ld.anchors.y == ld.anchors.w) {
        float desiredHeight;
        if(ld.size.y == 0) {
            desiredHeight = canUseAspect && info.maintainAspect && info.desiredSize.x != 0
                ? info.desiredSize.y / info.desiredSize.x * (max.x - min.x)
                : info.desiredSize.y;
        } else {
            desiredHeight = ld.size.y;
        }
        
        min.y -= desiredHeight * ld.origin.y;
        max.y += desiredHeight * (1.f - ld.origin.y);

        min.y += ld.position.y;
        max.y += ld.position.y;
    }
    else
    {
        min.y += ld.margin.y;
        max.y -= ld.margin.w;
    }
}

void UIElement::setLayoutDetails(LayoutDetails details, bool markDirty)
{
    layoutDetails = details;
    if(markDirty) {
        markLayoutDirty();
    }
}

vec4 UIElement::adjustRect(vec4 rect) const
{
    vec2 rectMin(rect.x, rect.y);
    vec2 rectMax(rect.z, rect.w);
    vec2 rectWidth = rectMax - rectMin;
    
    vec2 min = vec2(layoutDetails.anchors.x, layoutDetails.anchors.y) * rectWidth + rectMin;
    vec2 max = vec2(layoutDetails.anchors.z, layoutDetails.anchors.w) * rectWidth + rectMin;

    bool widthDetermined = min.x != max.x || layoutDetails.size.x != 0;
    bool heightDetermined = min.y != max.y || layoutDetails.size.y != 0;

    UILayoutRequest info = getLayoutRequest();

    if(widthDetermined) {
        adjustX(min, max, layoutDetails, info, false);
        adjustY(min, max, layoutDetails, info, true);
    } else {
        adjustY(min, max, layoutDetails, info, false);
        adjustX(min, max, layoutDetails, info, true);
    }

    return vec4(min, max);
}

void UIElement::markLayoutDirty()
{
    layoutDirty = true;
    shared_ptr<UIElement> parent = parentElement.lock();
    if(parent) {
        parent->markLayoutDirty();
    }
}

void UIElement::addElementChild(shared_ptr<UIElement> newChild)
{
    if(!newChild) {
        throw "Attempting to make null a child of this element.";
    }
    shared_ptr<UIElement> currentParent = newChild->parentElement.lock();
    if(currentParent) {
        throw "Attempting to add element as child which already has a parent!";
    }
    newChild->parentElement = shared_from_this();
    children.push_back(newChild);
}

void UIElement::removeElementChild(shared_ptr<UIElement> child)
{
    auto it = find(children.begin(), children.end(), child);
    if(it == children.end()) {
        return;
    }

    releaseChild(child);
    child->parentElement = shared_ptr<UIElement>();
    children.erase(it);
}

void UIElement::clearElementChildren()
{
    for(int i = children.size() - 1; i >= 0; i--) {
        releaseChild(children[i]);
        children[i]->parentElement = shared_ptr<UIElement>();
        children.erase(children.end() - 1);
    }
}

bool UIElement::updateChildLayoutRequests()
{
    bool stillDirty = false;
    for(shared_ptr<UIElement>& child : children) {
        stillDirty = stillDirty || child->updateLayoutRequest();
    }
    return stillDirty;
}

bool UIElement::updateLayoutRequest()
{
    if(layoutDirty) {
        bool childrenDirty = updateChildLayoutRequests();
        auto p = computeLayoutRequest();
        layoutRequest = p.first;
        return childrenDirty || p.second;
    }
    return false;
}

bool UIElement::updateLayouts(vec4 newLayout, bool clearDirtyFlag)
{
    layoutBox = newLayout;
    bool stillDirty = false;
    for(auto p : computeChildLayouts()) {
        stillDirty = stillDirty || p.first->updateLayouts(p.second, clearDirtyFlag);
    }
    if(clearDirtyFlag && !stillDirty) {
        layoutDirty = false;
    }
    return layoutDirty;
}

shared_ptr<UIElement> UIElement::queryLayout(vec2 point, vec4 mask, bool onlyInteractive)
{
    if(onlyInteractive && !blocksInteractive) {
        return nullptr;
    }

    return isPointInBox(mask, point) && testPoint(point) ? shared_from_this() : nullptr;
}

pair<UILayoutRequest, bool> UIElement::computeLayoutRequest()
{
    UILayoutRequest req;
    req.desiredSize = vec2(0,0);
    req.maintainAspect = false;
    return make_pair(req, false);
}

bool UIElement::testPoint(vec2 point)
{
    return isPointInBox(layoutBox, point);
}

void UIElement::getAncestors(vector<shared_ptr<UIElement>>& ancestors)
{
    ancestors.push_back(shared_from_this());
    shared_ptr<UIElement> parent = parentElement.lock();
    if(parent) {
        parent->getAncestors(ancestors);
    }
}

vector<shared_ptr<UIElement>> UIElement::getAncestors()
{
    vector<shared_ptr<UIElement>> ancestors;
    getAncestors(ancestors);
    return move(ancestors);
}
