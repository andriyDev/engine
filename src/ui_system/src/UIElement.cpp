
#include "ui/UIElement.h"

void adjustX(vec2& min, vec2& max, const vec2& size, const vec4& margin, const vec2& origin, const vec2& position,
    const UILayoutInfo& info, bool canUseAspect)
{
    if(min.x == max.x) {
        float desiredWidth;
        if(size.x == 0) {
            desiredWidth = canUseAspect && info.maintainAspect && info.desiredSize.y != 0
                ? info.desiredSize.x / info.desiredSize.y * (max.y - min.y)
                : info.desiredSize.x;
        } else {
            desiredWidth = size.x;
        }
        
        min.x -= desiredWidth * origin.x;
        max.x += desiredWidth * (1.f - origin.x);

        min.x += position.x;
        max.x += position.x;
    } else {
        min.x += margin.x;
        max.x -= margin.z;
    }
}

void adjustY(vec2& min, vec2& max, const vec2& size, const vec4& margin, const vec2& origin, const vec2& position,
    const UILayoutInfo& info, bool canUseAspect)
{

    if(min.y == max.y) {
        float desiredHeight;
        if(size.y == 0) {
            desiredHeight = canUseAspect && info.maintainAspect && info.desiredSize.x != 0
                ? info.desiredSize.y / info.desiredSize.x * (max.x - min.x)
                : info.desiredSize.y;
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
}

vec4 UIElement::adjustRect(vec4 rect, const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const
{
    vec2 rectMin(rect.x, rect.y);
    vec2 rectMax(rect.z, rect.w);
    vec2 rectWidth = rectMax - rectMin;
    
    vec2 min = vec2(anchors.x, anchors.y) * rectWidth + rectMin;
    vec2 max = vec2(anchors.z, anchors.w) * rectWidth + rectMin;

    bool widthDetermined = min.x != max.x || size.x != 0;
    bool heightDetermined = min.y != max.y || size.y != 0;

    UILayoutInfo info = layoutInfo.find(this)->second;

    if(widthDetermined) {
        adjustX(min, max, size, margin, origin, position, info, false);
        adjustY(min, max, size, margin, origin, position, info, true);
    } else {
        adjustY(min, max, size, margin, origin, position, info, false);
        adjustX(min, max, size, margin, origin, position, info, true);
    }

    return vec4(min, max);
}
