// InputController v 1.2
#pragma once

#include <GLFW/glfw3.h>
#include <custom/Camera.h>

class Character;

class InputController {
private:
    Camera& controlledCamera_;
    Character* controlledCharacter_ = nullptr;

    float cameraSpeed_;

    float lastX_ = 400.0f;
    float lastY_ = 300.0f;
    bool firstMouse_ = true;
    float sensitivity_ = 0.1f;

public:
    InputController(Camera& camera, float speed, float sensitivity = 0.1f)
        : controlledCamera_(camera), cameraSpeed_(speed), sensitivity_(sensitivity) {
    }

    void setCharacter(Character* character) {
        controlledCharacter_ = character;
    }

    void processKeyboardInput(GLFWwindow* window, float deltaTime);
    void processMouseInput(GLFWwindow* window);
};