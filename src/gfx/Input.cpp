#include "common/gl_includes.hpp"
#include "gfx/Input.hpp"

static bool fPressedLastFrame = false;
bool flashlightOn = true; // definicja globalnej

void ProcessInput(GLFWwindow* window, Camera* camera, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboard(RIGHT,    deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera->ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)     camera->ProcessKeyboard(UP,   deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressedLastFrame) { flashlightOn = !flashlightOn; fPressedLastFrame = true; }
    } else {
        fPressedLastFrame = false;
    }
}