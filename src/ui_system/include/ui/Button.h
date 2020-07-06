
#pragma once

#include "ui/Box.h"

class Button : public Box
{
public:
    Button();

    vec4 defaultColour;
    vec4 hoveredColour;
    vec4 pressedColour;

    virtual void update(float delta, shared_ptr<UISystem> ui) override;
private:
    enum State : uint
    {
        Default = 0,
        Hovered = 1,
        Pressed = 2
    };

    State currentState = State::Default;

    vec4 getColourByState(State state);
};
