
#include "ui/Button.h"

#include "ui/UISystem.h"

Button::Button()
{
    blocksInteractive = true;
    canBeFocused = true;
}

void Button::update(float delta, shared_ptr<UISystem> ui)
{
    Container::update(delta, ui);

    shared_ptr<UIElement> focus = ui->getFocusedElement();

    switch (currentState)
    {
    case Default:
        if(focus.get() == this) {
            currentState = Hovered;
        }
        break;
    case Hovered:
        if(focus.get() != this) {
            currentState = Default;
        } else if(ui->isMousePressed(UISystem::MouseButton::LMB)) {
            currentState = PressedByMouse;
        } else if(ui->isAcceptPressed()) {
            currentState = PressedByAccept;
        }
        break;
    case PressedByMouse:
        if(focus.get() != this) {
            currentState = Default;
        } else if(!ui->isMouseDown(UISystem::MouseButton::LMB)) {
            currentState = Hovered;
        }
        break;
    case PressedByAccept:
        if(focus.get() != this) {
            currentState = Default;
        } else if(!ui->isAcceptDown()) {
            currentState = Hovered;
        }
        break;
    }
    
    colour = getColourByState(currentState);
}

vec4 Button::getColourByState(State state)
{
    switch(state)
    {
    case Default:
        return stateColours.base;
    case Hovered:
        return stateColours.hovered;
    default: // case PressedBy(Mouse/Accept)
        return stateColours.active;
    }
}
