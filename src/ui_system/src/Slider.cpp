
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
    UIElement::LayoutDetails ld = bar->getLayoutDetails();
    ld.padding = bar->cornerRadii;
    if(isHorizontal(direction)) {
        ld.margin.x = (buttonSize - sliderBarWidth) * 0.5f;
        ld.margin.z = (buttonSize - sliderBarWidth) * 0.5f;
        ld.origin.y = 0.5f;
        ld.position.y = 0;
        ld.size.y = sliderBarWidth;
        ld.anchors = vec4(0, 0.5f, 1, 0.5f);
    } else {
        ld.margin.y = (buttonSize - sliderBarWidth) * 0.5f;
        ld.margin.w = (buttonSize - sliderBarWidth) * 0.5f;
        ld.origin.x = 0.5f;
        ld.position.x = 0;
        ld.size.x = sliderBarWidth;
        ld.anchors = vec4(0.5f, 0, 0.5f, 1);
    }
    bar->setLayoutDetails(ld);

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
    ld = filledBar->getLayoutDetails();
    ld.anchors = vec4(0,0,1,1);
    ld.margin = -bar->cornerRadii;
    filledBar->setLayoutDetails(ld);

    if(!button) {
        button = make_shared<Box>();
        button->label = "sliderButton";
        ld = button->getLayoutDetails();
        ld.anchors = vec4(0, 0.5f, 0, 0.5f);
        ld.origin = vec2(0.5f, 0.5f);
        button->setLayoutDetails(ld);
        button->blocksInteractive = false;
        bar->addChild(button);
    }
    button->cornerRadii = vec4(1,1,1,1) * (buttonSize * .5f);
    ld = button->getLayoutDetails();
    ld.size = vec2(1,1) * buttonSize;
    button->setLayoutDetails(ld);
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
    UIElement::LayoutDetails ldButton = button->getLayoutDetails();
    UIElement::LayoutDetails ldBar = filledBar->getLayoutDetails();

    if(isHorizontal(direction)) {
        if(isReversed(direction)) {
            ldButton.anchors.x = 1.0f - value;
            ldBar.anchors.x = 1.0f - value;
        } else {
            ldButton.anchors.x = value;
            ldBar.anchors.z = value;
        }
        ldButton.anchors.z = ldButton.anchors.x;
    } else {
        if(isReversed(direction)) {
            ldButton.anchors.y = 1.0f - value;
            ldBar.anchors.y = 1.0f - value;
        } else {
            ldButton.anchors.y = value;
            ldBar.anchors.w = value;
        }
        ldButton.anchors.w = ldButton.anchors.y;
    }
    button->setLayoutDetails(ldButton, false);
    filledBar->setLayoutDetails(ldBar, false);
    bar->updateLayouts(bar->getLayoutBox(), false);
}
