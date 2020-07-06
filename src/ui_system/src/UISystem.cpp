
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
}
