#ifndef GRAPRA_INPUTMANAGER_H
#define GRAPRA_INPUTMANAGER_H

#include "InputBinding.h"
#include <cppgl.h>

class InputManager {
private:
    InputManager() = default;
    ~InputManager() = default;

public:
    /**
     * Changes the input bindings of the Singleton.
     * A reference to the Singleton is returned.
     */
    static InputManager& changeBindings(const std::vector<InputBinding> &bindings){
        instance().doChangeBindings(bindings);
        return instance();
    }
    /**
     * Returns reference to the Singleton.
     */
    static InputManager& instance(){
        static InputManager inputManager;
        return inputManager;
    }

public:
    void update() {
        for (auto &[keyCode, pair] : keyboardPressMap) {
            if (!pair.first && Context::key_pressed(keyCode)) {
                // GLFW_PRESS
                pair.first = true;
                pair.second = true;
            } else if (pair.first) {
                if (!Context::key_pressed(keyCode)) {
                    // GLFW_RELEASE
                    pair.first = false;
                }
                // GLFW_RELEASE || GLFW_REPEAT
                pair.second = false;
            }
        }
        for (auto &[keyCode, pair]: mousePressMap) {
            if (!pair.first && Context::mouse_button_pressed(keyCode)) {
                // GLFW_PRESS
                pair.first = true;
                pair.second = true;
            } else if (pair.first) {
                if (!Context::mouse_button_pressed(keyCode)) {
                    // GLFW_RELEASE
                    pair.first = false;
                }
                // GLFW_RELEASE || GLFW_REPEAT
                pair.second = false;
            }
        }

        if (captureMouseMovement) {
            const glm::vec2 newPos = Context::mouse_pos();
            const glm::vec2 diff = mousePos - newPos;
            if (glm::length(diff) > 0.01) {
                mouseDelta = diff;
                mousePos = newPos;
            } else {
                mouseDelta = {0, 0};
            }
        }
        if(captureScrollMovement){
            if (glm::length(newScrollDelta) > 0.01) {
                scrollDelta = newScrollDelta;
                scrollPos += newScrollDelta;
            } else {
                scrollDelta = {0, 0};
            }
            newScrollDelta = {0,0};
        }
    }

    [[nodiscard]] bool isActive(InputBinding binding) const {
        assertNotMovement(binding);

        if (binding.device == KEYBOARD && binding.mode == HOLD) {
            return Context::key_pressed(binding.keyCode);
        }
        if (binding.device == MOUSE && binding.mode == HOLD) {
            return Context::mouse_button_pressed(binding.keyCode);
        }

        if (binding.device == KEYBOARD && binding.mode == PRESS) {
            return keyboardPressMap.at(binding.keyCode).second;
        }
        if (binding.device == MOUSE && binding.mode == PRESS) {
            return mousePressMap.at(binding.keyCode).second;
        }

        notImplemented();
        return false;
    }

    [[nodiscard]] bool hasMoved(InputBinding binding) const {
        assertIsMovement(binding);

        if (binding.device == MOUSE && binding.mode == MOVE) {
            return mouseDelta != glm::vec2(0);
        }
        if (binding.device == MOUSE && binding.mode == SCROLL) {
            return scrollDelta != glm::vec2(0);
        }

        notImplemented();
        return false;
    }
    [[nodiscard]] bool hasMovedX(InputBinding binding) const {
        assertIsMovement(binding);

        if (binding.device == MOUSE && binding.mode == MOVE) {
            return mouseDelta.x != 0;
        }
        if (binding.device == MOUSE && binding.mode == SCROLL) {
            return scrollDelta.x != 0;
        }

        notImplemented();
        return false;
    }
    [[nodiscard]] bool hasMovedY(InputBinding binding) const {
        assertIsMovement(binding);

        if (binding.device == MOUSE && binding.mode == MOVE) {
            return mouseDelta.y != 0;
        }
        if (binding.device == MOUSE && binding.mode == SCROLL) {
            return scrollDelta.y != 0;
        }

        notImplemented();
        return false;
    }
    [[nodiscard]] glm::vec2 getMovement(InputBinding binding) const {
        assertIsMovement(binding);

        if (binding.device == MOUSE && binding.mode == MOVE) {
            return mouseDelta;
        }
        if (binding.device == MOUSE && binding.mode == SCROLL) {
            return scrollDelta;
        }

        notImplemented();
        return {0,0};
    }
    [[nodiscard]] glm::vec2 getPos(InputBinding binding) const {
        assertIsMovement(binding);

        if (binding.device == MOUSE && binding.mode == MOVE) {
            return mousePos;
        }
        if (binding.device == MOUSE && binding.mode == SCROLL) {
            return scrollPos;
        }

        notImplemented();
        return {0,0};
    }

protected:
    void doChangeBindings(const std::vector<InputBinding> &bindings){
        // reset

        keyboardPressMap = {};
        mousePressMap = {};

        captureMouseMovement = false;
        mousePos = {0,0}, mouseDelta = {0,0};

        Context::set_mouse_scroll_callback(nullptr);
        captureScrollMovement = false;
        scrollPos = {0,0}, scrollDelta = {0,0};
        newScrollDelta = {0,0};

        // init

        for (const auto &binding: bindings) {
            if(binding.mode == HOLD){
                continue;
            }

            if (binding.device == KEYBOARD && binding.mode == PRESS) {
                keyboardPressMap[binding.keyCode] = {false, false};
                continue;
            }
            if (binding.device == MOUSE && binding.mode == PRESS) {
                mousePressMap[binding.keyCode] = {false, false};
                continue;
            }

            if (binding.device == MOUSE && binding.mode == MOVE) {
                captureMouseMovement = true;
                continue;
            }
            if (binding.device == MOUSE && binding.mode == SCROLL) {
                captureScrollMovement = true;
                // There is glfwGetCursorPos() for mouse position but not glfwGetScrollPos().
                // Thus, we need to use a callback.
                Context::set_mouse_scroll_callback(scrollCallback);
                continue;
            }

            notImplemented();
        }
    }

    static void scrollCallback(double deltaX, double deltaY){
        instance().newScrollDelta += glm::vec2{deltaX,deltaY};
    }

    static void assertNotMovement(InputBinding binding) {
        if (binding.mode == MOVE || binding.mode == SCROLL) {
            throw std::invalid_argument("expected no movement");
        }
    }

    static void assertIsMovement(InputBinding binding) {
        if (binding.mode != MOVE && binding.mode != SCROLL) {
            throw std::invalid_argument("expected a movement");
        }
    }

    static void notImplemented(){
        throw std::runtime_error("not implemented");
    }

protected:
    /**
     * map[keyCode, [hold, press]]
     */
    std::map<int, std::pair<bool, bool>> keyboardPressMap;
    std::map<int, std::pair<bool, bool>> mousePressMap;

    bool captureMouseMovement = false;
    glm::vec2 mousePos = {0,0}, mouseDelta = {0,0};

    bool captureScrollMovement = false;
    glm::vec2 scrollPos = {0,0}, scrollDelta = {0,0};
    glm::vec2 newScrollDelta = {0,0};
};

#endif //GRAPRA_INPUTMANAGER_H
