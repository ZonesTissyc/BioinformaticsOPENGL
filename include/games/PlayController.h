#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Character;
class Camera;

class PlayController {
public:
    PlayController(Character* character, Camera& camera, float moveSpeed = 2.5f);

    // 每帧处理输入
    void processInput(GLFWwindow* window, float deltaTime);

private:
    Character* controlledCharacter_ = nullptr;
    Camera& controlledCamera_;
    float moveSpeed_;

    // 偏移，让摄像机高于头部一点
    glm::vec3 cameraOffset_ = glm::vec3(0.00f, 0.0050f, 0.030f);

    // 内部方法
    void handleMovement(GLFWwindow* window, float deltaTime);
    void handleActions(GLFWwindow* window);
    void updateCameraToHead();  // 每帧绑定摄像机到头骨
};
