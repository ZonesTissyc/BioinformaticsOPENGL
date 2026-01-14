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
    
    // 处理鼠标输入（用于控制角色转向）
    void processMouseInput(float xoffset, float yoffset, float sensitivity = 0.1f);
    
    // 设置移动速度
    void setMoveSpeed(float speed) { moveSpeed_ = speed; }
    
    // 获取当前移动速度
    float getMoveSpeed() const { return moveSpeed_; }

private:
    Character* controlledCharacter_ = nullptr;
    Camera& controlledCamera_;
    float moveSpeed_;

    // 偏移，让摄像机高于头部一点
    glm::vec3 cameraOffset_ = glm::vec3(0.00f, 0.006f, 0.0120f);

    // 鼠标左键上一帧的状态（用于检测按下瞬间）
    bool leftMousePressedLast_ = false;

    // 内部方法
    void handleMovement(GLFWwindow* window, float deltaTime);
    void handleActions(GLFWwindow* window);
    void updateCameraToHead();  // 每帧绑定摄像机到头骨
};
