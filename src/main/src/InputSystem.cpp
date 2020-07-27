
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
    charsTyped = "";
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

string InputSystem::consumeCharTyped()
{
    string out = charsTyped;
    charsTyped = "";
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

uchar mapKeyToChar(int key, int mods)
{
    switch(key) {
    case GLFW_KEY_ENTER:
        return '\n';
    case GLFW_KEY_BACKSPACE:
        return 8; // Backspace in ASCII.
    case GLFW_KEY_TAB:
        return 9; // Tab in ASCII.
    case GLFW_KEY_DELETE:
        return 127; // DEL in ASCII.
    case GLFW_KEY_LEFT:
        return 17; // Device Control 1 in ASCII.
    case GLFW_KEY_RIGHT:
        return 18; // Device Control 2 in ASCII.
    case GLFW_KEY_DOWN:
        return 19; // Device Control 3 in ASCII.
    case GLFW_KEY_UP:
        return 20; // Device Control 4 in ASCII.
    case GLFW_KEY_PAGE_UP:
        return 128;
    case GLFW_KEY_PAGE_DOWN:
        return 129;
    case GLFW_KEY_HOME:
        return 130;
    case GLFW_KEY_END:
        return 131;
    }
    return 0;
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

    uchar val = mapKeyToChar(key, mods);
    if(val) { charsTyped += val; }
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

    uchar val = mapKeyToChar(key, mods);
    if(val) { charsTyped += val; }
}

void InputSystem::charTyped(uint character)
{
    charsTyped += (uchar)character;
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

void InputSystem::createAction(uint controlSet, const string& actionName)
{
    controlSets[controlSet].actions.insert_or_assign(actionName, ControlSet::Action());
}

void InputSystem::addActionKeyBind(uint controlSet, const string& actionName,
    int key, bool ctrl, bool alt, bool shift, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    int mods = (ctrl ? GLFW_MOD_CONTROL : 0) | (alt ? GLFW_MOD_ALT : 0) | (shift ? GLFW_MOD_SHIFT : 0);
    it->second.controls.push_back(make_tuple(weight, (uint)key, mods));
}
    
void InputSystem::addActionMouseBind(uint controlSet, const string& actionName, int mouseButton, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    it->second.controls.push_back(make_tuple(weight, (uint)(MOUSE_BTNS + mouseButton), 0));
}

void InputSystem::addActionSpecialMouseBind(uint controlSet, const string& actionName,
    int specialMouseId, float weight)
{
    auto it = controlSets[controlSet].actions.find(actionName);
    assert(it != controlSets[controlSet].actions.end());
    it->second.controls.push_back(make_tuple(weight, (uint)specialMouseId, 0));
}
    

float InputSystem::getActionValue(uint controlSet, const string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.getValue(*this, isGameplayTick);
}

bool InputSystem::isActionPressed(uint controlSet, const string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isPressed(*this, isGameplayTick);
}

bool InputSystem::isActionReleased(uint controlSet, const string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isReleased(*this, isGameplayTick);
}

bool InputSystem::isActionDown(uint controlSet, const string& actionName, bool isGameplayTick)
{
    assert(controlSets.size() > controlSet);
    auto it = controlSets[controlSet].actions.find(actionName);
    if(it == controlSets[controlSet].actions.end()) {
        return 0;
    }
    return it->second.isDown(*this, isGameplayTick);
}

bool InputSystem::isKeyPressed(uint key)
{
    return keys_frame[key].pressed;
}

bool InputSystem::isKeyReleased(uint key)
{
    return keys_frame[key].released;
}

bool InputSystem::isKeyDown(uint key)
{
    return keys_frame[key].value > 0;
}

bool InputSystem::isKeyPressedGameplay(uint key)
{
    return keys_gameplay[key].pressed;
}

bool InputSystem::isKeyReleasedGameplay(uint key)
{
    return keys_gameplay[key].released;
}

bool InputSystem::isKeyDownGameplay(uint key)
{
    return keys_gameplay[key].value > 0;
}

bool InputSystem::isMousePressed(uint btn)
{
    return keys_frame[MOUSE_BTNS + btn].pressed;
}

bool InputSystem::isMouseReleased(uint btn)
{
    return keys_frame[MOUSE_BTNS + btn].released;
}

bool InputSystem::isMouseDown(uint btn)
{
    return keys_frame[MOUSE_BTNS + btn].value > 0;
}

bool InputSystem::isMousePressedGameplay(uint btn)
{
    return keys_gameplay[MOUSE_BTNS + btn].pressed;
}

bool InputSystem::isMouseReleasedGameplay(uint btn)
{
    return keys_gameplay[MOUSE_BTNS + btn].released;
}

bool InputSystem::isMouseDownGameplay(uint btn)
{
    return keys_gameplay[MOUSE_BTNS + btn].value > 0;
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
        value += get<0>(control)
            * (isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[get<1>(control)].value
            * (IS.areModsDown(get<2>(control), isGameplayTick) ? 1 : 0);
    }
    return value;
}

bool InputSystem::ControlSet::Action::isPressed(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[get<1>(control)].pressed
            && IS.areModsDown(get<2>(control), isGameplayTick)) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isReleased(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[get<1>(control)].released
            && IS.areModsDown(get<2>(control), isGameplayTick)) {
            return true;
        }
    }
    return false;
}

bool InputSystem::ControlSet::Action::isDown(const InputSystem& IS, bool isGameplayTick) const
{
    for(auto control : controls) {
        if((isGameplayTick ? IS.keys_gameplay : IS.keys_frame)[get<1>(control)].value > 0
            && IS.areModsDown(get<2>(control), isGameplayTick)) {
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

