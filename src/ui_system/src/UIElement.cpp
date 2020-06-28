
#include "ui/UIElement.h"

vec4 UIElement::adjustRect(vec4 rect, const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const
{
    vec2 rectMin(rect.x, rect.y);
    vec2 rectMax(rect.z, rect.w);
    vec2 rectWidth = rectMax - rectMin;
    
    vec2 min = vec2(anchors.x, anchors.y) * rectWidth + rectMin;
    vec2 max = vec2(anchors.z, anchors.w) * rectWidth + rectMin;

    if(min.x == max.x) {
        float desiredWidth;
        if(size.x == 0) {
            desiredWidth = layoutInfo.find(this)->second.desiredSize.x;
        } else {
            desiredWidth = size.x;
        }
        
        min.x -= desiredWidth * origin.x;
        max.x += desiredWidth * (1.f - origin.x);

        min.x += position.x;
        max.x += position.x;
    }
    else
    {
        min.x += margin.x;
        max.x -= margin.z;
    }

    if(min.y == max.y) {
        float desiredHeight;
        if(size.y == 0) {
            desiredHeight = layoutInfo.find(this)->second.desiredSize.y;
        } else {
            desiredHeight = size.y;
        }
        
        min.y -= desiredHeight * origin.y;
        max.y += desiredHeight * (1.f - origin.y);

        min.y += position.y;
        max.y += position.y;
    }
    else
    {
        min.y += margin.y;
        max.y -= margin.w;
    }

    return vec4(min, max);
}
