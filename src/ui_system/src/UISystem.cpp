
#include "ui/UISystem.h"

#define GLEW_STATIC
#include <GL/glew.h>

void UISystem::frameTick(float delta)
{
    glDisable(GL_DEPTH_TEST);
    hash_map<const UIElement*, vec2> desiredSizes;
    for(const shared_ptr<UIElement>& element : elements) {
        element->layout(desiredSizes);
    }
    for(const shared_ptr<UIElement>& element : elements) {
        element->render(glm::vec4(0,0,1,1), glm::vec4(0,0,1,1), desiredSizes);
    }
    glEnable(GL_DEPTH_TEST);
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
