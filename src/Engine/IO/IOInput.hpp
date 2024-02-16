#pragma once
#include "pch.hpp"

enum class EIOInputType {
    KEYBOARD,
    MOUSE_BUTTON,
    MOUSE_MOVE,
    MOUSE_SCROLL,
};

enum class EKeyboardKeys {
    W,
    S,
    A,
    D,
    Q,
    E,
    R,
    X,
    L,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SPACE,
    TAB,
    F5,
    UNKNOWN
};

enum class EKeyAction {
    PRESS,
    RELEASE,
    REPEAT,
    UNKNOWN
};

enum class EMouseButton {
    LEFT,
    RIGHT,
    MIDDLE,
};

struct IOInput {
    EIOInputType type;
    std::optional<double> x, y;
    std::optional<EMouseButton> button;
    std::optional<EKeyboardKeys> key;
    std::optional<int> scancode; // platform-specific scancode
    std::optional<EKeyAction> action;
    std::optional<int> mods; // bitfield
};
