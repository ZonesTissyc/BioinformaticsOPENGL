#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <custom/Camera.h>

class Character;
class PlayController;

class InputController {
public:
    enum class ControlTarget { Camera, Character };

    InputController(Camera& camera, float speed, float sensitivity = 0.1f);

    void setCharacter(Character* character);
    void setPlayController(PlayController* controller);

    void processKeyboardInput(GLFWwindow* window, float deltaTime);
    void processMouseInput(GLFWwindow* window);

private:
    void processCameraInput(GLFWwindow* window, float deltaTime);
    void processCharacterInput(GLFWwindow* window, float deltaTime);

private:
    Camera& controlledCamera_;
    Character* controlledCharacter_ = nullptr;
    PlayController* playController_ = nullptr;

    ControlTarget currentTarget_ = ControlTarget::Camera;

    float cameraSpeed_;
    float sensitivity_ = 0.1f;

    bool firstMouse_ = true;
    float lastX_ = 400.0f;
    float lastY_ = 300.0f;
};
