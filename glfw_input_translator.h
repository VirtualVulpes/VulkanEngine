#pragma once
#include "input_manager.h"

namespace glfw {
    Key toKey(int glfwKey);
    InputAction toAction(int glfwAction);
    MouseButton toMouseButton(int glfwMouseButton);
}


