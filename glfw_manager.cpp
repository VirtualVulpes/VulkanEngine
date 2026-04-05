#include "glfw_manager.h"

#include <GLFW/glfw3.h>

GLFWManager::GLFWManager()
{
    if (!glfwInit())
    {
        throw std::runtime_error("failed to initialize glfw");
    }
}

GLFWManager::~GLFWManager()
{
    glfwTerminate();
}
