
#pragma once

#include "std.h"
#include "core/System.h"
#include "Window.h"

#define SCROLL_X_POS 24
#define SCROLL_X_NEG 25
#define SCROLL_Y_POS 26
#define SCROLL_Y_NEG 27
#define MOUSE_X_POS 28
#define MOUSE_X_NEG 29
#define MOUSE_Y_POS 30
#define MOUSE_Y_NEG 31

class InputSystem : public System, public WindowEventHandler
{
public:
    virtual void frameTick(float delta) override;
    virtual void gameplayTick(float delta) override;

    virtual void keyPressed(int key, int scancode, int mods) override;
    virtual void keyReleased(int key, int scancode, int mods) override;
    virtual void keyRepeated(int key, int scancode, int mods) override;
    virtual void charTyped(uint character) override;
    virtual void mousePressed(int button, int mods) override;
    virtual void mouseReleased(int button, int mods) override;
    virtual void mouseMoved(double x, double y) override;
    virtual void mouseEntered() override;
    virtual void mouseExited() override;
    virtual void mouseScroll(double xScroll, double yScroll) override;

    void setControlSetCount(uint count);
    void createAction(uint controlSet, const string& actionName);
    void addActionKeyBind(uint controlSet, const string& actionName,
        int key, bool ctrl, bool alt, bool shift, float weight = 1);
    void addActionMouseBind(uint controlSet, const string& actionName, int mouseButton, float weight = 1);
    void addActionSpecialMouseBind(uint controlSet, const string& actionName,
        int specialMouseId, float weight = 1);

    float getActionValue(uint controlSet, const string& actionName, bool useGameplayTick);
    bool isActionPressed(uint controlSet, const string& actionName, bool useGameplayTick);
    bool isActionReleased(uint controlSet, const string& actionName, bool useGameplayTick);
    bool isActionDown(uint controlSet, const string& actionName, bool useGameplayTick);

    bool isKeyPressed(uint key);
    bool isKeyReleased(uint key);
    bool isKeyDown(uint key);

    bool isKeyPressedGameplay(uint key);
    bool isKeyReleasedGameplay(uint key);
    bool isKeyDownGameplay(uint key);
    
    bool isMousePressed(uint btn);
    bool isMouseReleased(uint btn);
    bool isMouseDown(uint btn);
    
    bool isMousePressedGameplay(uint btn);
    bool isMouseReleasedGameplay(uint btn);
    bool isMouseDownGameplay(uint btn);

    void setCursor(bool lock, bool hidden);

    void setTargetWindow(Window* _target);
    uint consumeCharTyped();
    vec2 consumeMouseDelta();
    float consumeScrollDeltaX();
    float consumeScrollDeltaY();

    inline vec2 getMousePosition() const { return mousePosition; }
    inline bool isMouseOnWindow() const { return mouseOnWindow; }
private:
    struct KeyState
    {
        float value;
        bool pressed;
        bool released;
        bool repeated;
    };
    struct ControlSet
    {
    private:
        struct Action
        {
            float getValue(InputSystem& IS, bool isGameplayTick) const;
            bool isPressed(const InputSystem& IS, bool isGameplayTick) const;
            bool isReleased(const InputSystem& IS, bool isGameplayTick) const;
            bool isDown(const InputSystem& IS, bool isGameplayTick) const;

            /* The controls for this action stored as (weight, key, modifiers). */
            vector<tuple<float, uint, int>> controls;
        };

        hash_map<string, Action> actions;

        friend class InputSystem;
    };

    bool areModsDown(int mods, bool isGameplayTick) const;

    bool mouseOnWindow = false;
    KeyState keys_gameplay[GLFW_KEY_LAST + 1];
    KeyState keys_frame[GLFW_KEY_LAST + 1];
    uint lastCharTyped = 0;
    vec2 mousePosition;
    vec2 mouseDelta;
    vec2 scrollDelta;
    vector<ControlSet> controlSets;
    Window* targetWindow;
};
