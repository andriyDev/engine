
#include "InputSystem.h"

#define MOUSE_BTNS 1

const uint SPECIAL_KEYS[9] = {
    SCROLL_X_NEG, SCROLL_X_POS, SCROLL_Y_NEG, SCROLL_Y_POS,
    MOUSE_X_NEG, MOUSE_X_POS, MOUSE_Y_NEG, MOUSE_Y_POS,
    0
};

void InputSystem::frameTick(float delta)
{
    for(int i = 0; i <= GLFW_KEY_LAST; i++) {
        keys_frame[i].pressed = false;
        keys_frame[i].released = false;
        keys_frame[i].repeated = false;
    }
    for(int i = 0; SPECIAL_KEYS[i]; i++) {
        keys_frame[SPECIAL_KEYS[i]].value = 0;
    }
    lastCharTyped = 0;
    mouseDelta = {0, 0};
    scrollDelta = {0, 0};
    if(targetWindow) {
        targetWindow->poll();
    }
}

void InputSystem::gameplayTick(float delta)
{
    for(int i = 0; i <= GLFW_KEY_LAST; i++) {
        keys_gameplay[i].pressed = false;
        keys_gameplay[i].released = false;
        keys_gameplay[i].repeated = false;
    }
    for(int i = 0; SPECIAL_KEYS[i]; i++) {
        keys_gameplay[SPECIAL_KEYS[i]].value = 0;
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
    keys_frame[key].pressed = true;
    keys_frame[key].value = 1;
    keys_gameplay[key].pressed = true;
    keys_gameplay[key].value = 1;
}

void InputSystem::keyReleased(int key, int scancode, int mods)
{
    if(key == GLFW_KEY_UNKNOWN) {
        return;
    }
    keys_frame[key].released = true;
    keys_frame[key].value = 0;
    keys_gameplay[key].released = true;
    keys_gameplay[key].value = 0;
}

void InputSystem::keyRepeated(int key, int scancode, int mods)
{
    if(key == GLFW_KEY_UNKNOWN) {
        return;
    }
    keys_frame[key].repeated = true;
    keys_gameplay[key].repeated = true;
}

void InputSystem::charTyped(uint character)
{
    lastCharTyped = character;
}

void InputSystem::mousePressed(int button, int mods)
{
    keys_frame[button + MOUSE_BTNS].pressed = true;
    keys_frame[button + MOUSE_BTNS].value = 1;
    keys_gameplay[button + MOUSE_BTNS].pressed = true;
    keys_gameplay[button + MOUSE_BTNS].value = 1;
}

void InputSystem::mouseReleased(int button, int mods)
{
    keys_frame[button + MOUSE_BTNS].released = true;
    keys_frame[button + MOUSE_BTNS].value = 0;
    keys_gameplay[button + MOUSE_BTNS].released = true;
    keys_gameplay[button + MOUSE_BTNS].value = 0;
}

void InputSystem::mouseMoved(double x, double y)
{
    mouseDelta += glm::vec2(x, y) - mousePosition;
    mousePosition = {x, y};

    if(mouseDelta.x >= 0) {
        keys_frame[MOUSE_X_POS].value = mouseDelta.x;
        keys_frame[MOUSE_X_NEG].value = 0;
        keys_gameplay[MOUSE_X_POS].value = mouseDelta.x;
        keys_gameplay[MOUSE_X_NEG].value = 0;
    } else {
        keys_frame[MOUSE_X_NEG].value = -mouseDelta.x;
        keys_frame[MOUSE_X_POS].value = 0;
        keys_gameplay[MOUSE_X_NEG].value = -mouseDelta.x;
        keys_gameplay[MOUSE_X_POS].value = 0;
    }

    if(mouseDelta.y >= 0) {
        keys_frame[MOUSE_Y_POS].value = mouseDelta.y;
        keys_frame[MOUSE_Y_NEG].value = 0;
        keys_gameplay[MOUSE_Y_POS].value = mouseDelta.y;
        keys_gameplay[MOUSE_Y_NEG].value = 0;
    } else {
        keys_frame[MOUSE_Y_NEG].value = -mouseDelta.y;
        keys_frame[MOUSE_Y_POS].value = 0;
        keys_gameplay[MOUSE_Y_NEG].value = -mouseDelta.y;
        keys_gameplay[MOUSE_Y_POS].value = 0;
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
    scrollDelta += glm::vec2(xScroll, yScroll);
    
    if(scrollDelta.x >= 0) {
        keys_frame[SCROLL_X_POS].value = scrollDelta.x;
        keys_frame[SCROLL_X_NEG].value = 0;
        keys_gameplay[SCROLL_X_POS].value = scrollDelta.x;
        keys_gameplay[SCROLL_X_NEG].value = 0;
    } else {
        keys_frame[SCROLL_X_NEG].value = -scrollDelta.x;
        keys_frame[SCROLL_X_POS].value = 0;
        keys_gameplay[SCROLL_X_NEG].value = -scrollDelta.x;
        keys_gameplay[SCROLL_X_POS].value = 0;
    }

    if(scrollDelta.y >= 0) {
        keys_frame[SCROLL_Y_POS].value = scrollDelta.y;
        keys_frame[SCROLL_Y_NEG].value = 0;
        keys_gameplay[SCROLL_Y_POS].value = scrollDelta.y;
        keys_gameplay[SCROLL_Y_NEG].value = 0;
    } else {
        keys_frame[SCROLL_Y_NEG].value = -scrollDelta.y;
        keys_frame[SCROLL_Y_POS].value = 0;
        keys_gameplay[SCROLL_Y_NEG].value = -scrollDelta.y;
        keys_gameplay[SCROLL_Y_POS].value = 0;
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
    

float InputSystem::getActionValue(uint controlSet, const std::string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.getValue(*this, isGameplayTick);
}

bool InputSystem::isActionPressed(uint controlSet, const std::string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isPressed(*this, isGameplayTick);
}

bool InputSystem::isActionReleased(uint controlSet, const std::string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isReleased(*this, isGameplayTick);
}

bool InputSystem::isActionDown(uint controlSet, const std::string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isDown(*this, isGameplayTick);
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

float InputSystem::ControlSet::Action::getValue(InputSystem& IS, bool isGameplayTick) const
{
    float value = 0;
    for(auto control : controls) {
        value += std::get<0>(control)
            * (isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[std::get<1>(control)].value
            * (IS.areModsDown(std::get<2>(control), isGameplayTick) ? 1 : 0);
    }
    return value;
}

bool InputSystem::ControlSet::Action::isPressed(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[std::get<1>(control)].pressed
            && IS.areModsDown(std::get<2>(control), isGameplayTick)) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isReleased(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[std::get<1>(control)].released
            && IS.areModsDown(std::get<2>(control), isGameplayTick)) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isDown(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[std::get<1>(control)].value > 0
            && IS.areModsDown(std::get<2>(control), isGameplayTick)) {
            return true;
        }
    }
    return false;
}

bool InputSystem::areModsDown(int mods, bool isGameplayTick) const
{
    const KeyState* keys = (isGameplayTick ? keys_gameplay : keys_frame);
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

