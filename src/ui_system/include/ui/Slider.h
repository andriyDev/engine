
#pragma once

#include "std.h"
#include "ui/WrapperElement.h"
#include "ui/Box.h"
#include "UIUtil.h"

class Slider : public WrapperElement
{
public:
    Slider();

    enum Direction
    {
        Horizontal,
        Vertical,
        HorizontalReverse,
        VerticalReverse
    };

    Direction direction = Horizontal;
    float buttonSize = 15.f;
    float sliderBarWidth = 5.f;
    vec4 sliderBarColour = vec4(0.6f, 0.6f, 0.6f, 1.0f);
    UIColour buttonColour = { vec4(0.8f, 0.8f, 0.8f, 1.0f), vec4(1, 1, 1, 1), vec4(0.5f, 0.5f, 0.5f, 1) };
    vec4 filledBarColour = vec4(0.7f, 0.7f, 0.7f, 1.0f);

    float value = 0.25f;

    void sync();

    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) override;
protected:
    enum State : uint
    {
        Default = 0,
        Hovered = 1,
        Dragging = 2
    };

    State currentState = Default;

    shared_ptr<Box> bar;
    shared_ptr<Box> filledBar;
    shared_ptr<Box> button;

    friend UIColour::Type convert(State state);
};
