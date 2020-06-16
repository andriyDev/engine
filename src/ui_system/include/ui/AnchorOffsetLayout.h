
#pragma once

#include "std.h"

#include "UIElement.h"

class AnchorOffsetLayout : public UIElement
{
public:
    struct AnchorOffset
    {
        vec2 anchorMin;
        vec2 anchorMax;

        union {
            vec2 offsetMin;
            vec2 position;
        };
        union {
            vec2 offsetMax;
            vec2 size;
        };
        vec2 origin;

        vec4 getMargin() const {
            vec4 m = vec4(0,0,0,0);
            if(anchorMin.x != anchorMax.x) {
                m.x = offsetMin.x;
                m.z = offsetMax.x;
            }
            if(anchorMin.y != anchorMax.y) {
                m.y = offsetMin.y;
                m.w = offsetMax.y;
            }
            return m;
        }
    };

    void addChild(const AnchorOffset& slot, shared_ptr<UIElement> element);
    void removeChild(shared_ptr<UIElement> element);
    void clearChildren();
    AnchorOffset* getSlot(shared_ptr<UIElement> element);

    virtual vec2 layout(hash_map<const UIElement*, vec2>& desiredSizes) override;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, vec2>& desiredSizes) override;
protected:
    vector<pair<AnchorOffset, shared_ptr<UIElement>>> children;
};
