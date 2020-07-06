
#pragma once

#include "std.h"

#include "core/System.h"
#include "renderer/RenderSurface.h"

#include "UIElement.h"

class UISystem : public System
{
    virtual void init() override;
    virtual void frameTick(float delta) override;
public:
    RenderSurface* targetSurface = nullptr;
    bool swapBuffers = true;
    float uiScale = 1.0f;

    vec2 getMousePoint() const { return mousePoint; }
    shared_ptr<UIElement> getTopElement() const { return topElement.lock(); }
    shared_ptr<UIElement> getTopInteractiveElement() const { return topInteractiveElement.lock(); }

    enum class MouseButton : uchar
    {
        LMB = 0,
        RMB = 1
    };

    void clearMouseStates();
    void onMousePressed(MouseButton btn);
    void onMouseReleased(MouseButton btn);
    void onMouseMove(vec2 newMousePoint);

    bool isMousePressed(MouseButton btn) const { return mousePressed[(uchar)btn]; }
    bool isMouseDown(MouseButton btn) const { return mouseDown[(uchar)btn]; }
    bool isMouseReleased(MouseButton btn) const { return mouseReleased[(uchar)btn]; }

    void addElement(shared_ptr<UIElement> element);
    void removeElement(shared_ptr<UIElement> element);

    void setDefaultFocus(shared_ptr<UIElement> element);
    void focusElement(shared_ptr<UIElement> element);
    shared_ptr<UIElement> changeFocus(UIElement::Direction direction);
    shared_ptr<UIElement> getFocusedElement() const { return focusedElement.lock(); }
private:
    vector<shared_ptr<UIElement>> elements;
    weak_ptr<UIElement> topElement;
    weak_ptr<UIElement> topInteractiveElement;
    vec2 lastSurfaceSize = vec2(0,0);
    vec2 mousePoint;
    bool mouseHasMoved = false;
    weak_ptr<UIElement> focusedElement;
    weak_ptr<UIElement> defaultFocus;

    bool mousePressed[2];
    bool mouseReleased[2];
    bool mouseDown[2];

    void updateMouseData();
    void updateTopElements();
};
