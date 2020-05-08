
#include "InputSystem.h"

#define MOUSE_BTNS 1

void InputSystem::frameTick(float delta, float tickPercent)
{
    for(int i = 0; i <= GLFW_KEY_LAST; i++) {
        keys[i].pressed = false;
        keys[i].released = false;
        keys[i].repeated = false;
    }
    lastCharTyped = 0;
    mouseDelta = {0, 0};
    scrollDelta = {0, 0};
    if(targetWindow) {
        targetWindow->poll();
    }
}

uint InputSystem::consumeCharTyped()
{
    uint out = lastCharTyped;
    lastCharTyped = 0;
    return out;
}

glm::vec2 InputSystem::consumeMouseDelta()
{
    glm::vec2 out = mouseDelta;
    mouseDelta = {0, 0};
    return out;
}

float InputSystem::consumeScrollDeltaX()
{
    float out = scrollDelta.x;
    scrollDelta.x = 0;
    return out;
}

float InputSystem::consumeScrollDeltaY()
{
    float out = scrollDelta.y;
    scrollDelta.y = 0;
    return out;
}

void InputSystem::keyPressed(int key, int scancode, int mods)
{
    if(key == GLFW_KEY_UNKNOWN) {
        return;
    }
    keys[key].pressed = true;
    keys[key].value = 1;
}

void InputSystem::keyReleased(int key, int scancode, int mods)
{
    if(key == GLFW_KEY_UNKNOWN) {
        return;
    }
    keys[key].released = true;
    keys[key].value = 0;
}

void InputSystem::keyRepeated(int key, int scancode, int mods)
{
    if(key == GLFW_KEY_UNKNOWN) {
        return;
    }
    keys[key].repeated = true;
}

void InputSystem::charTyped(uint character)
{
    lastCharTyped = character;
}

void InputSystem::mousePressed(int button, int mods)
{
    keys[button + MOUSE_BTNS].pressed = true;
    keys[button + MOUSE_BTNS].value = 1;
}

void InputSystem::mouseReleased(int button, int mods)
{
    keys[button + MOUSE_BTNS].released = true;
    keys[button + MOUSE_BTNS].value = 0;
}

void InputSystem::mouseMoved(double x, double y)
{
    mouseDelta += glm::vec2(x, y) - mousePosition;
    mousePosition = {x, y};

    if(mouseDelta.x >= 0) {
        keys[MOUSE_X_POS].value = mouseDelta.x;
        keys[MOUSE_X_NEG].value = 0;
    } else {
        keys[MOUSE_X_NEG].value = -mouseDelta.x;
        keys[MOUSE_X_POS].value = 0;
    }

    if(mouseDelta.y >= 0) {
        keys[MOUSE_Y_POS].value = mouseDelta.y;
        keys[MOUSE_Y_NEG].value = 0;
    } else {
        keys[MOUSE_Y_NEG].value = -mouseDelta.y;
        keys[MOUSE_Y_POS].value = 0;
    }
}

void InputSystem::mouseEntered()
{
    mouseOnWindow = true;
}

void InputSystem::mouseExited()
{
    mouseOnWindow = false;
}

void InputSystem::mouseScroll(double xScroll, double yScroll)
{
    scrollDelta = {xScroll, yScroll};
    
    if(scrollDelta.x >= 0) {
        keys[SCROLL_X_POS].value = scrollDelta.x;
        keys[SCROLL_X_NEG].value = 0;
    } else {
        keys[SCROLL_X_NEG].value = -scrollDelta.x;
        keys[SCROLL_X_POS].value = 0;
    }

    if(scrollDelta.y >= 0) {
        keys[SCROLL_Y_POS].value = scrollDelta.y;
        keys[SCROLL_Y_NEG].value = 0;
    } else {
        keys[SCROLL_Y_NEG].value = -scrollDelta.y;
        keys[SCROLL_Y_POS].value = 0;
    }
}

void InputSystem::setControlSetCount(uint count)
{
    controlSets.resize(count);
}

void InputSystem::createAction(uint controlSet, const std::string& actionName)
{
    controlSets[controlSet].actions.insert_or_assign(actionName, ControlSet::Action());
}

void InputSystem::addActionKeyBind(uint controlSet, const std::string& actionName,
    int key, bool ctrl, bool alt, bool shift, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    int mods = (ctrl ? GLFW_MOD_CONTROL : 0) | (alt ? GLFW_MOD_ALT : 0) | (shift ? GLFW_MOD_SHIFT : 0);
    it->second.controls.push_back(std::make_tuple(weight, (uint)key, mods));
}
    
void InputSystem::addActionMouseBind(uint controlSet, const std::string& actionName, int mouseButton, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    it->second.controls.push_back(std::make_tuple(weight, (uint)(MOUSE_BTNS + mouseButton), 0));
}

void InputSystem::addActionSpecialMouseBind(uint controlSet, const std::string& actionName,
    int specialMouseId, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    it->second.controls.push_back(std::make_tuple(weight, (uint)specialMouseId, 0));
}
    

float InputSystem::getActionValue(uint controlSet, const std::string& actionName, bool consume)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.getValue(*this, consume);
}

bool InputSystem::isActionPressed(uint controlSet, const std::string& actionName)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isPressed(*this);
}

bool InputSystem::isActionReleased(uint controlSet, const std::string& actionName)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isReleased(*this);
}

bool InputSystem::isActionDown(uint controlSet, const std::string& actionName)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isDown(*this);
}


void InputSystem::setCursor(bool lock, bool hidden)
{
    if(targetWindow) {
        targetWindow->setCursor(lock, hidden);
    }
}

void InputSystem::setTargetWindow(Window* _target)
{
    if(targetWindow) {
        targetWindow->removeEventHandler(this);
    }
    targetWindow = _target;
    if(targetWindow) {
        targetWindow->addEventHandler(this);
    }
}

float InputSystem::ControlSet::Action::getValue(InputSystem& IS, bool consume) const
{
    float value = 0;
    for(auto control : controls) {
        //std::cout << (char)std::get<1>(control) << " " << IS.keys[std::get<1>(control)].value << std::endl;
        value += std::get<0>(control) * IS.keys[std::get<1>(control)].value
            * (IS.areModsDown(std::get<2>(control)) ? 1 : 0);
        if(consume) {
            uint key = std::get<1>(control);
            if(key == SCROLL_X_POS || key == SCROLL_X_NEG) {
                IS.keys[std::get<1>(control)].value = 0;
                IS.scrollDelta.x = 0;
            } else if(key == SCROLL_Y_POS || key == SCROLL_Y_NEG) {
                IS.keys[std::get<1>(control)].value = 0;
                IS.scrollDelta.y = 0;
            } else if(key == MOUSE_X_POS || key == MOUSE_X_NEG) {
                IS.keys[std::get<1>(control)].value = 0;
                IS.mouseDelta.x = 0;
            } else if(key == MOUSE_Y_POS || key == MOUSE_Y_NEG) {
                IS.keys[std::get<1>(control)].value = 0;
                IS.mouseDelta.y = 0;
            }
        }
    }
    return value;
}

bool InputSystem::ControlSet::Action::isPressed(const InputSystem& IS) const
{
    for(auto control : controls) {
        if(IS.keys[std::get<1>(control)].pressed && IS.areModsDown(std::get<2>(control))) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isReleased(const InputSystem& IS) const
{
    for(auto control : controls) {
        if(IS.keys[std::get<1>(control)].released && IS.areModsDown(std::get<2>(control))) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isDown(const InputSystem& IS) const
{
    for(auto control : controls) {
        if(IS.keys[std::get<1>(control)].value > 0 && IS.areModsDown(std::get<2>(control))) {
            return true;
        }
    }
    return false;
}

bool InputSystem::areModsDown(int mods) const
{
    if(mods & GLFW_MOD_SHIFT) {
        if(keys[GLFW_KEY_LEFT_SHIFT].value == 0 && keys[GLFW_KEY_RIGHT_SHIFT].value == 0) {
            return false;
        }
    }
    if(mods & GLFW_MOD_CONTROL) {
        if(keys[GLFW_KEY_LEFT_CONTROL].value == 0 && keys[GLFW_KEY_RIGHT_CONTROL].value == 0) {
            return false;
        }
    }
    if(mods & GLFW_MOD_ALT) {
        if(keys[GLFW_KEY_LEFT_ALT].value == 0 && keys[GLFW_KEY_RIGHT_ALT].value == 0) {
            return false;
        }
    }
    return true;
}

