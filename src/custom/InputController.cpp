#include <custom/InputController.h>
#include <games/character.h>
#include <games/PlayController.h>
#include <glm/glm.hpp>

InputController::InputController(Camera& camera, float speed, float sensitivity)
    : controlledCamera_(camera), cameraSpeed_(speed), sensitivity_(sensitivity) {
}

void InputController::setCharacter(Character* character) {
    controlledCharacter_ = character;
}

void InputController::setPlayController(PlayController* controller) {
    playController_ = controller;
}

void InputController::processKeyboardInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 切换控制目标
    static bool f1PressedLast = false;
    bool f1PressedNow = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
    if (f1PressedNow && !f1PressedLast) {
        currentTarget_ = (currentTarget_ == ControlTarget::Camera)
            ? ControlTarget::Character
            : ControlTarget::Camera;
    }
    f1PressedLast = f1PressedNow;

    // 根据目标分发输入
    if (currentTarget_ == ControlTarget::Camera) {
        processCameraInput(window, deltaTime);
    }
    else if (currentTarget_ == ControlTarget::Character) {
        processCharacterInput(window, deltaTime);
    }
}

void InputController::processMouseInput(GLFWwindow* window) {
    if (currentTarget_ != ControlTarget::Camera) return; // 鼠标只影响摄像机

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

void InputController::processCameraInput(GLFWwindow* window, float deltaTime) {
    float velocity = cameraSpeed_ * deltaTime;

    glm::vec3 pos = controlledCamera_.getPos();
    glm::vec3 front = controlledCamera_.getFront();
    glm::vec3 right = controlledCamera_.getRight();
    glm::vec3 up = controlledCamera_.getUp();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) pos += up * velocity;

    controlledCamera_.setPos(pos);
}

void InputController::processCharacterInput(GLFWwindow* window, float deltaTime) {
    if (!controlledCharacter_ || !playController_) return;
    playController_->processInput(window, deltaTime);
}
