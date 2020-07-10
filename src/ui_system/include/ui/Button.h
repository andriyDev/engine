
#pragma once

#include "std.h"
#include "ui/Box.h"
#include "ui/UIUtil.h"

class Button : public Box
{
public:
    Button();

    UIColour stateColours;

    virtual void update(float delta, vec4 mask, shared_ptr<UISystem> ui) override;
private:
    enum State : uint
    {
        Default = 0,
        Hovered = 1,
        PressedByMouse = 2,
        PressedByAccept = 3
    };

    State currentState = State::Default;

    vec4 getColourByState(State state);
};
