
#include "ui/Button.h"

#include "ui/UISystem.h"

Button::Button()
{
    blocksInteractive = true;
}

void Button::update(float delta, shared_ptr<UISystem> ui)
{
    Container::update(delta, ui);

    shared_ptr<UIElement> topElement = ui->getTopInteractiveElement();

    switch (currentState)
    {
    case Default:
        if(topElement.get() == this) {
            currentState = Hovered;
        }
        break;
    case Hovered:
        if(topElement.get() != this) {
            currentState = Default;
        } else if(ui->isMousePressed(UISystem::MouseButton::LMB)) {
            currentState = Pressed;
        }
        break;
    case Pressed:
        if(topElement.get() != this) {
            currentState = Default;
        } else if(ui->isMouseReleased(UISystem::MouseButton::LMB)) {
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
        return defaultColour;
    case Hovered:
        return hoveredColour;
    default: // case Pressed
        return pressedColour;
    }
}
