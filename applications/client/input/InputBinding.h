#ifndef GRAPRA_INPUTBINDING_H
#define GRAPRA_INPUTBINDING_H

#include <cppgl.h>

enum InputDevice {
    KEYBOARD,
    MOUSE,
};

enum InputMode {
    /**
     * Once when key is pressed
     */
    PRESS,
    /**
     * As long as the key is being pressed
     */
    HOLD,
    /**
     * The delta that has been moved
     */
    MOVE,
    /**
     * The delta that has been scrolled
     */
    SCROLL,
};

class InputBinding {
public:
    InputBinding(InputDevice device, InputMode mode, int keyCode)
            : device(device), mode(mode), keyCode(keyCode) {}

    const InputDevice device;
    const InputMode mode;
    const int keyCode;
};

/**
 * The default input bindings
 */
class InputBindings {
public:
    InputBindings() = default;

    [[nodiscard]] std::vector<InputBinding> asList() const {
        return {left,
                right,
                forward,
                backward,
                run,
                crawl,
                jump,
                //
                toggle_camera,
                pause,
                act1,
                act2,
                //
                mouse,
                scroll};
    }

    // HOLD
    InputBinding left{KEYBOARD, HOLD, GLFW_KEY_A};
    InputBinding right{KEYBOARD,HOLD, GLFW_KEY_D};
    InputBinding forward{KEYBOARD, HOLD,GLFW_KEY_W};
    InputBinding backward{KEYBOARD,HOLD, GLFW_KEY_S};
    InputBinding run{KEYBOARD,HOLD, GLFW_KEY_LEFT_SHIFT};
    InputBinding crawl{KEYBOARD, HOLD,GLFW_KEY_LEFT_CONTROL};
    InputBinding jump{KEYBOARD, HOLD, GLFW_KEY_SPACE};
    InputBinding act1{MOUSE, HOLD, GLFW_MOUSE_BUTTON_LEFT};
    // PRESS
    InputBinding toggle_camera{KEYBOARD, PRESS, GLFW_KEY_C};
    InputBinding pause{KEYBOARD, PRESS, GLFW_KEY_ESCAPE};
    InputBinding act2{MOUSE, PRESS, GLFW_MOUSE_BUTTON_RIGHT};
    // MOVE/SCROLL
    InputBinding mouse{MOUSE, MOVE, GLFW_KEY_UNKNOWN};
    InputBinding scroll{MOUSE, SCROLL, GLFW_KEY_UNKNOWN};
};

#endif //GRAPRA_INPUTBINDING_H
