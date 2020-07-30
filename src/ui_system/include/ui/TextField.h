
#pragma once

#include "std.h"
#include "ui/WrapperElement.h"
#include "ui/Text.h"
#include "ui/Box.h"
#include "ui/UIUtil.h"
#include "ui/UISystem.h"

class TextField : public WrapperElement, public KeyTypeListener
{
public:
    TextField();

    vec4 cornerRadii = vec4(5.0f, 5.0f, 5.0f, 5.0f);
    vec4 interiorPadding = vec4(5.0f, 5.0f, 5.0f, 5.0f);
    ResourceRef<Font> font;
    UIColour backgroundColour = {
        vec4(0.75f, 0.75f, 0.75f, 1.0f),
        vec4(0.9f, 0.9f, 0.9f, 1.0f),
        vec4(0.65f, 0.65f, 0.65f, 1.0f)
    };
    UIColour textColour = {
        vec4(0,0,0,1),
        vec4(0,0,0,1),
        vec4(0,0,0,1)
    };
    vec4 dragColour = vec4(45.0f/255, 144.0f/255, 214.0f/255, 0.5f);
    bool useUnboundedLayout = true;
    Font::Alignment alignment = Font::Alignment::Left;
    float pulseRate = 0.5f;
    float pulseWidth = 1.0f;
    float fontSize = 15.0f;

    void sync();

    void setValue(string _value);
    void setTextPoint(uint point, bool selectRange = false);

    void selectTextPoint(uint point);

    void movePointerLeft(bool select);
    void movePointerRight(bool select);
    void movePointerLeftToken(bool select);
    void movePointerRightToken(bool select);
    void movePointerDown(bool select);
    void movePointerUp(bool select);
    void movePointerPageUp(bool select);
    void movePointerPageDown(bool select);
    void movePointerHome(bool select);
    void movePointerEnd(bool select);
    void backspaceSelected();
    void deleteSelected();

    string getValue() const { return value; }
    uint getTextPoint() const { return textPoints.y; }

    virtual void render(vec4 mask, vec2 surfaceSize) override;
    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) override;

    virtual void onKeyTyped(uint key, shared_ptr<UISystem> ui) override;
protected:
    enum State : uint
    {
        Default = 0,
        Hovered = 1,
        Dragging = 2,
        Typing = 3
    };

    State currentState = Default;

    string value;
    uvec2 textPoints = uvec2(0, 0);

    shared_ptr<Box> box;
    shared_ptr<Container> textBox;
    shared_ptr<Text> text;
    shared_ptr<Box> textPulse;
    shared_ptr<Box> dragBox;

    float pulseTime = 0.f;
    bool pulsing = true;

    vec2 textOffset = vec2(0,0);

    uint getTextPointFromPosition(vec2 position) const;

    void renderDragBoxes(vec4 mask, vec2 surfaceSize);

    friend UIColour::Type convert(State state);
};
