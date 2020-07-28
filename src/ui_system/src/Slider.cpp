
#include "ui/Slider.h"
#include "ui/Container.h"
#include "ui/Box.h"
#include "ui/Button.h"
#include "ui/UISystem.h"

Slider::Slider()
{
    blocksInteractive = true;
    canBeFocused = true;
};

#define isHorizontal(dir) ((dir) == Slider::Horizontal || (dir) == Slider::HorizontalReverse)
#define isReversed(dir) ((dir) == Slider::HorizontalReverse || (dir) == Slider::VerticalReverse)

void Slider::sync()
{
    shared_ptr<Container> base;
    if(wrappedElements.size() > 0) {
        base = static_pointer_cast<Container>(wrappedElements[0]);
    } else {
        base = make_shared<Container>();
        base->label = "baseContainer";
        base->layoutAlgorithm = new OverlayLayout();
        wrappedElements.push_back(base);
    }

    if(!bar) {
        bar = make_shared<Box>();
        bar->label = "bar";
        bar->blocksInteractive = false;
        bar->layoutAlgorithm = new OverlayLayout();
        bar->maskChildren = false;
        base->addChild(bar);
    }
    bar->colour = sliderBarColour;
    bar->cornerRadii = vec4(1,1,1,1) * (sliderBarWidth * .5f);
    bar->padding = bar->cornerRadii;
    if(isHorizontal(direction)) {
        bar->margin.x = (buttonSize - sliderBarWidth) * 0.5f;
        bar->margin.z = (buttonSize - sliderBarWidth) * 0.5f;
        bar->origin.y = 0.5f;
        bar->position.y = 0;
        bar->size.y = sliderBarWidth;
        bar->anchors = vec4(0, 0.5f, 1, 0.5f);
    } else {
        bar->margin.y = (buttonSize - sliderBarWidth) * 0.5f;
        bar->margin.w = (buttonSize - sliderBarWidth) * 0.5f;
        bar->origin.x = 0.5f;
        bar->position.x = 0;
        bar->size.x = sliderBarWidth;
        bar->anchors = vec4(0.5f, 0, 0.5f, 1);
    }

    if(!filledBar) {
        filledBar = make_shared<Box>();
        filledBar->label = "barFill";
        filledBar->blocksInteractive = false;
        bar->addChild(filledBar);
    }
    filledBar->colour = filledBarColour;
    switch(direction) {
    case Horizontal:
        filledBar->cornerRadii = vec4(1,0,1,0);
        break;
    case Vertical:
        filledBar->cornerRadii = vec4(1,1,0,0);
        break;
    case HorizontalReverse:
        filledBar->cornerRadii = vec4(0,1,0,1);
        break;
    default:
        filledBar->cornerRadii = vec4(0,0,1,1);
    }
    filledBar->cornerRadii *= (sliderBarWidth * .5f);
    filledBar->anchors = vec4(0,0,1,1);
    filledBar->margin = -bar->padding;

    if(!button) {
        button = make_shared<Box>();
        button->label = "sliderButton";
        button->anchors = vec4(0, 0.5f, 0, 0.5f);
        button->origin = vec2(0.5f, 0.5f);
        button->blocksInteractive = false;
        bar->addChild(button);
    }
    button->cornerRadii = vec4(1,1,1,1) * (buttonSize * .5f);
    button->size = vec2(1,1) * buttonSize;
    button->colour = buttonColour.base;
}

UIColour::Type convert(Slider::State state)
{
    switch(state)
    {
    case Slider::Default:
        return UIColour::Base;
    case Slider::Hovered:
        return UIColour::Hovered;
    default:
        return UIColour::Active;
    }
}

void Slider::update(float delta, vec4 mask, shared_ptr<UISystem> ui)
{
    WrapperElement::update(delta, mask, ui);

    shared_ptr<UIElement> focus = ui->getFocusedElement();

    switch(currentState)
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
            currentState = Dragging;
            ui->focusLocked = true;
        }
        break;
    case Dragging:
        if(!ui->isMouseDown(UISystem::MouseButton::LMB)) {
            currentState = Hovered;
            ui->focusLocked = false;
        } else {
            float mousePos = ui->getMousePoint()[!isHorizontal(direction)];
            vec4 box = getLayoutBox();
            vec2 boxAlongAxis = isHorizontal(direction) ? vec2(box.x, box.z) : vec2(box.y, box.w);
            boxAlongAxis += vec2(1.f,-1.f) * (buttonSize * 0.5f);
            float normalizedValue = boxAlongAxis.x == boxAlongAxis.y ? 0
                : (mousePos - boxAlongAxis.x) / (boxAlongAxis.y - boxAlongAxis.x);
            normalizedValue = clamp(normalizedValue, 0.f, 1.f);
            value = isReversed(direction) ? 1 - normalizedValue : normalizedValue;
        }
    }

    button->colour = buttonColour.byType(convert(currentState));

    // Update the button (and filled bar) to match the new value.

    if(isHorizontal(direction)) {
        if(isReversed(direction)) {
            button->anchors.x = 1.0f - value;
            filledBar->anchors.x = 1.0f - value;
        } else {
            button->anchors.x = value;
            filledBar->anchors.z = value;
        }
        button->anchors.z = button->anchors.x;
    } else {
        if(isReversed(direction)) {
            button->anchors.y = 1.0f - value;
            filledBar->anchors.y = 1.0f - value;
        } else {
            button->anchors.y = value;
            filledBar->anchors.w = value;
        }
        button->anchors.w = button->anchors.y;
    }
    bar->updateLayouts(bar->getLayoutBox(), false);
}
