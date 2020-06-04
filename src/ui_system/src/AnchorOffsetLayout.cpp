
#include "ui/AnchorOffsetLayout.h"

void AnchorOffsetLayout::addChild(const AnchorOffset& slot, shared_ptr<UIElement> element)
{
    children.push_back(make_pair(slot, element));
}

void AnchorOffsetLayout::removeChild(shared_ptr<UIElement> element)
{
    for(auto it = children.begin(); it != children.end(); ++it) {
        if(it->second == element) {
            children.erase(it);
        }
    }
}

void AnchorOffsetLayout::clearChildren()
{
    children.clear();
}

AnchorOffsetLayout::AnchorOffset* AnchorOffsetLayout::getSlot(shared_ptr<UIElement> element)
{
    for(auto it = children.begin(); it != children.end(); ++it) {
        if(it->second == element) {
            return &it->first;
        }
    }
    return nullptr;
}

vec2 AnchorOffsetLayout::layout(hash_map<const UIElement*, vec2>& desiredSizes) const
{
    for(const auto& child : children) {
        vec2 size = child.second->layout(desiredSizes);
    }
    desiredSizes.insert(make_pair(this, vec2(0,0)));
    return vec2(0,0);
}

void AnchorOffsetLayout::render(vec4 rect, vec4 mask,
    const hash_map<const UIElement*, vec2>& desiredSizes)
{
    vec2 rectMin(rect.x, rect.y);
    vec2 rectMax(rect.z, rect.w);
    vec2 rectWidth = rectMax - rectMin;
    for(const auto& child : children) {
        vec2 min = child.first.anchorMin * rectWidth + rectMin;
        vec2 max = child.first.anchorMax * rectWidth + rectMin;

        if(min.x == max.x) {
            float desiredWidth;
            if(child.first.size.x == 0) {
                desiredWidth = desiredSizes.find(child.second.get())->second.x;
            } else {
                desiredWidth = child.first.size.x;
            }
            
            min.x -= desiredWidth * child.first.origin.x;
            max.x += desiredWidth * (1.f - child.first.origin.x);

            min.x += child.first.position.x;
            max.x += child.first.position.x;
        }
        else
        {
            min.x += child.first.offsetMin.x;
            max.x -= child.first.offsetMax.x;
        }

        if(min.y == max.y) {
            float desiredHeight;
            if(child.first.size.y == 0) {
                desiredHeight = desiredSizes.find(child.second.get())->second.y;
            } else {
                desiredHeight = child.first.size.y;
            }
            
            min.y -= desiredHeight * child.first.origin.y;
            max.y += desiredHeight * (1.f - child.first.origin.y);

            min.y += child.first.position.y;
            max.y += child.first.position.y;
        }
        else
        {
            min.y += child.first.offsetMin.y;
            max.y -= child.first.offsetMax.y;
        }

        child.second->render(vec4(min, max), mask, desiredSizes);
    }
}
