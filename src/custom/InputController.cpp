// InputController v 1.2
#include <custom/InputController.h>
#include <games/character.h>
#include <glm/glm.hpp>

void InputController::processKeyboardInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float velocity = cameraSpeed_ * deltaTime;

    glm::vec3 pos = controlledCamera_.getPos();
    glm::vec3 front = controlledCamera_.getFront();
    glm::vec3 right = controlledCamera_.getRight();
    glm::vec3 up = controlledCamera_.getUp();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        pos += front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        pos -= front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        pos += right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        pos -= right * velocity;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        pos += up * velocity;
    }

    controlledCamera_.setPos(pos);

    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
    {
        if (controlledCharacter_)
        {
            controlledCharacter_->PlayAnimation("death", true);
        }
    }
}

void InputController::processMouseInput(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse_) {
        lastX_ = static_cast<float>(xpos);
        lastY_ = static_cast<float>(ypos);
        firstMouse_ = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX_;
    float yoffset = lastY_ - static_cast<float>(ypos);

    lastX_ = static_cast<float>(xpos);
    lastY_ = static_cast<float>(ypos);

    xoffset *= sensitivity_;
    yoffset *= sensitivity_;

    controlledCamera_.processMouseMovement(xoffset, yoffset);
}
