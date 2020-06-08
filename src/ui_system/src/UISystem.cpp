
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
    hash_map<const UIElement*, vec2> desiredSizes;
    for(const shared_ptr<UIElement>& element : elements) {
        element->layout(desiredSizes);
    }
    vec2 surfaceSize = targetSurface->getSize();
    for(const shared_ptr<UIElement>& element : elements) {
        element->render(
            vec4(0,0,surfaceSize.x, surfaceSize.y),
            vec4(0,0,surfaceSize.x,surfaceSize.y),
            surfaceSize, desiredSizes);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    if(swapBuffers) {
        targetSurface->swapBuffers();
    }
}

void UISystem::addElement(shared_ptr<UIElement> element)
{
    elements.insert(element);
}

void UISystem::removeElement(shared_ptr<UIElement> element)
{
    elements.erase(element);
}
