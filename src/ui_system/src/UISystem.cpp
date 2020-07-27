
#include "ui/UISystem.h"

#define GLEW_STATIC
#include <GL/glew.h>

KeyTypeListener::~KeyTypeListener()
{
    shared_ptr<UISystem> sys = system.lock();
    if(!sys) {
        return;
    }
    sys->removeTypingListener(this);
}

void UISystem::init()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UISystem::frameTick(float delta)
{
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

    updateForced = true;
    while(updateForced) {
        updateForced = false;
        if(mouseHasMoved) {
            updateTopElements();
            mouseHasMoved = false;
        }
        
        for(const shared_ptr<UIElement>& element : elements) {
            element->update(delta, screenBox, static_pointer_cast<UISystem>(shared_from_this()));
        }
        // After the first update, any subsequent updates will have no time expire.
        delta = 0;
    }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
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

void UISystem::onCharacterTyped(uint character)
{
    for(KeyTypeListener* listener : listeners) {
        listener->onKeyTyped(character, static_pointer_cast<UISystem>(shared_from_this()));
    }
}

void UISystem::addTypingListener(KeyTypeListener* listener)
{
    if(!listener) { return; }
    listeners.insert(listener);
    listener->system = static_pointer_cast<UISystem>(shared_from_this());
}

void UISystem::removeTypingListener(KeyTypeListener* listener)
{
    if(listeners.erase(listener)) {
        listener->system = shared_ptr<UISystem>(nullptr);
    }
}

void UISystem::updateKey(uint key, bool pressed, bool down, bool released)
{
    keys[key].pressed = pressed;
    keys[key].down = down;
    keys[key].released = released;
}

void UISystem::updateTopElements()
{
    if(focusLocked) {
        return;
    }
    
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

    if(_topInteractiveElement && _topInteractiveElement->canBeFocused) {
        if(_topInteractiveElement != focusedElement.lock()) {
            focusElement(_topInteractiveElement);
        }
    } else {
        focusElement(nullptr);
    }
}

void UISystem::setDefaultFocus(shared_ptr<UIElement> element)
{
    defaultFocus = element;
}

void UISystem::focusElement(shared_ptr<UIElement> element)
{
    if(focusLocked) {
        return;
    }
    focusedElement = element;
}

shared_ptr<UIElement> UISystem::changeFocus(UIElement::Direction direction)
{
    shared_ptr<UIElement> currentFocus = focusedElement.lock();
    if(focusLocked) {
        return currentFocus;
    }
    
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

void UISystem::forceUpdate()
{
    mouseHasMoved = true;
    updateForced = true;
}
