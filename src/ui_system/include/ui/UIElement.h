
#pragma once

#include "std.h"

class UISystem;

struct UILayoutRequest
{
    vec2 desiredSize;
    bool maintainAspect = false;
};

class UIElement : public enable_shared_from_this<UIElement>
{
public:
    enum Gravity {
        Start, End, Center, Fill
    };
    enum Direction {
        Left = 0, Right = 1, Up = 2, Down = 3
    };
    struct LayoutDetails {
        // We package these together to save memory since both options cannot be used at the same time.
        union {
            struct {
                // The point to expand from relative to itself (0 means left/top, 1 means right/bottom).
                vec2 origin;
                // The position of the origin if the element is free.
                vec2 position;
            };
            // The space between an element and its parent (going left, top, right, bottom).
            vec4 margin = vec4(0,0,0,0);
        };
        // If we are free to choose our size, what should it be? 0 means use the desiredSize.
        vec2 size = vec2(0,0);
        // The points to anchor this element relative to the space it occupies (0 meaning left/top, 1 meaning right/bottom).
        vec4 anchors = vec4(0,0,1,1);
        // The space between an element and its children (going left, top, right, bottom).
        vec4 padding = vec4(0,0,0,0);
        // If the element is free in the horizontal direction, how should the element prefer to be positioned.
        Gravity horizontalGravity = Fill;
        // If the element is free in the vertical direction, how should the element prefer to be positioned.
        Gravity verticalGravity = Fill;
        // If space (or lack of space) must be distributed between siblings, how much should be given to this element.
        float weight = 0;
        // Minimum size this element wants (may not be provided).
        vec2 minSize = vec2(0,0);
        // Maximum size this element wants (may not be provided).
        vec2 maxSize = vec2(INFINITY,INFINITY);
    };

    void setLayoutDetails(LayoutDetails details, bool markDirty = true);
    inline const LayoutDetails& getLayoutDetails() { return layoutDetails; }

    const char* label = "";

    bool blocksInteractive = false;
    bool canBeFocused = false;

    weak_ptr<UIElement> neighbours[4];

    void markLayoutDirty();

    bool isLayoutDirty() const { return layoutDirty; }

    vec4 adjustRect(vec4 rect) const;

    // Updates the layout request of the element. Returns a bool indicating whether the layout request is still dirty.
    bool updateLayoutRequest();
    /* Updates the layout of the element. Also given a bool on whether to reset the dirty flag or not.
        Returns a flag whether still dirty.
    */
    bool updateLayouts(vec4 newLayout, bool clearDirtyFlag);

    // Sets the parent of this element. Should only be called by containers.
    void setParent(shared_ptr<UIElement> element);

    UILayoutRequest getLayoutRequest() const { return layoutRequest; }
    vec4 getLayoutBox() const { return layoutBox; }
    vec4 getPaddedLayoutBox() const;
    
    virtual void render(vec4 mask, vec2 surfaceSize) = 0;

    virtual shared_ptr<UIElement> queryLayout(vec2 point, vec4 mask, bool onlyInteractive = true);

    virtual bool testPoint(vec2 point);

    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) {}

    void getAncestors(vector<shared_ptr<UIElement>>& ancestors);
    vector<shared_ptr<UIElement>> getAncestors();
protected:
    /*
    Computes the layout request for this element (children all have valid layout requests).
    Returns the layout request and a bool indicating whether the layout request is still dirty.
    */
    virtual pair<UILayoutRequest, bool> computeLayoutRequest() = 0;
    // Computes a layout for each child.
    virtual hash_map<UIElement*, vec4> computeChildLayouts() { return hash_map<UIElement*, vec4>(); }
    // Calls updateLayoutRequest on each child of this element. Returns a bool which is an OR of the resulting calls.
    virtual bool updateChildLayoutRequests() { return false; }

    virtual void releaseChild(shared_ptr<UIElement> element) {}

    LayoutDetails layoutDetails;
private:
    bool layoutDirty = true;
    UILayoutRequest layoutRequest;
    vec4 layoutBox;
    weak_ptr<UIElement> parentElement;
};

inline vec4 intersect_boxes(vec4 box1, vec4 box2)
{
    return vec4(max(box1.x, box2.x), max(box1.y, box2.y), min(box1.z, box2.z), min(box1.w, box2.w));
}

inline bool isPointInBox(vec4 box, vec2 point)
{
    return box.x <= point.x && point.x <= box.z && box.y <= point.y && point.y <= box.w;
}
