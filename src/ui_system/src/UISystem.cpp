
#include "ui/UISystem.h"

#define GLEW_STATIC
#include <GL/glew.h>

void UISystem::init()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UISystem::frameTick(float delta)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    vec2 surfaceSize = targetSurface->getSize() * uiScale;
    bool relayout = false;
    if(surfaceSize != lastSurfaceSize) {
        lastSurfaceSize = surfaceSize;
        relayout = true;
    }
    vec4 screenBox = vec4(0,0, surfaceSize.x, surfaceSize.y);
    for(const shared_ptr<UIElement>& element : elements) {
        if(relayout || element->isLayoutDirty()) {
            bool stillDirty = element->updateLayoutRequest();
            element->updateLayouts(screenBox, !stillDirty);
        }
    }
    if(mouseHasMoved) {
        updateTopElements();
    }
    
    for(const shared_ptr<UIElement>& element : elements) {
        element->update(delta, static_pointer_cast<UISystem>(shared_from_this()));
    }
    for(const shared_ptr<UIElement>& element : elements) {
        element->render(screenBox, surfaceSize);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    if(swapBuffers) {
        targetSurface->swapBuffers();
    }
}

void UISystem::addElement(shared_ptr<UIElement> element)
{
    elements.push_back(element);
}

void UISystem::removeElement(shared_ptr<UIElement> element)
{
    auto it = find(elements.begin(), elements.end(), element);
    if(it != elements.end()) {
        elements.erase(it);
    }
}

void UISystem::clearInputStates()
{
    mousePressed[0] = false;
    mousePressed[1] = false;
    mouseReleased[0] = false;
    mouseReleased[1] = false;

    acceptPressed = false;
    acceptReleased = false;
}

void UISystem::onMousePressed(MouseButton btn)
{
    mousePressed[(uchar)btn] = true;
    mouseDown[(uchar)btn] = true;
}

void UISystem::onMouseReleased(MouseButton btn)
{
    mouseDown[(uchar)btn] = false;
    mouseReleased[(uchar)btn] = true;
}

void UISystem::onAcceptPressed()
{
    acceptPressed = true;
    acceptDown = true;
}

void UISystem::onAcceptReleased()
{
    acceptReleased = true;
    acceptDown = false;
}

void UISystem::onMouseMove(vec2 newMousePoint)
{
    newMousePoint *= uiScale;
    if(mousePoint != newMousePoint) {
        mouseHasMoved = true;
        mousePoint = newMousePoint;
    }
}

void UISystem::updateTopElements()
{
    shared_ptr<UIElement> _topInteractiveElement;
    shared_ptr<UIElement> _topElement;

    for(auto it = elements.rbegin(); it != elements.rend() && (!_topInteractiveElement || !_topElement); ++it) {
        shared_ptr<UIElement>& element = *it;
        if(!_topInteractiveElement) {
            _topInteractiveElement = element->queryLayout(mousePoint, vec4(0.f, 0.f, lastSurfaceSize), true);
        }
        if(!_topElement) {
            _topElement = element->queryLayout(mousePoint, vec4(0.f, 0.f, lastSurfaceSize), false);
        }
    }
    topElement = _topElement;
    topInteractiveElement = _topInteractiveElement;
}

void UISystem::setDefaultFocus(shared_ptr<UIElement> element)
{
    defaultFocus = element;
}

void UISystem::focusElement(shared_ptr<UIElement> element)
{
    focusedElement = element;
}

shared_ptr<UIElement> UISystem::changeFocus(UIElement::Direction direction)
{
    shared_ptr<UIElement> currentFocus = focusedElement.lock();
    if(!currentFocus) {
        currentFocus = defaultFocus.lock();
        focusedElement = currentFocus;
        return currentFocus;
    }
    shared_ptr<UIElement> directionElement = currentFocus->neighbours[(uint)direction].lock();
    if(directionElement) {
        focusedElement = directionElement;
        return directionElement;
    } else {
        return currentFocus;
    }
}
