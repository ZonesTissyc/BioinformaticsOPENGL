#include <games/PlayController.h>
#include <games/character.h>
#include <custom/Camera.h>
#include <custom/ModelTrans.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

PlayController::PlayController(Character* character, Camera& camera, float moveSpeed)
    : controlledCharacter_(character), controlledCamera_(camera), moveSpeed_(moveSpeed)
{
    // 初始化时更新角色的front向量
    if (controlledCharacter_) {
        updateCharacterFront();
    }
}

void PlayController::processInput(GLFWwindow* window, float deltaTime)
{
    if (!controlledCharacter_ || !controlledCharacter_->alive)
        return;

    // 确保front向量与yaw同步
    updateCharacterFront();
    
    handleMovement(window, deltaTime);
    handleActions(window);
    updateCameraToHead(); // 每帧同步摄像机
}

void PlayController::handleMovement(GLFWwindow* window, float deltaTime)
{
    glm::vec3 moveDirection(0.0f);

    // 获取相对于角色朝向的移动方向
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDirection += controlledCharacter_->front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDirection -= controlledCharacter_->front;
    
    // 计算右方向（相对于角色朝向）
    glm::vec3 right = glm::normalize(glm::cross(controlledCharacter_->front, glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDirection += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDirection -= right;

    if (glm::length(moveDirection) > 0.0f) {
        // 将Y分量设为0，只在水平面移动
        moveDirection.y = 0.0f;
        moveDirection = glm::normalize(moveDirection);
        controlledCharacter_->position += moveDirection * moveSpeed_ * deltaTime;

        // 切换 Run 动画
        controlledCharacter_->SetAction(Character::Action::Run);
    }
    else {
        // 切换 Idle 动画
        controlledCharacter_->SetAction(Character::Action::Stay);
    }
}

void PlayController::handleActions(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Attack, false);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Death, true);
        // controlledCharacter_->alive = false;
    }
}

void PlayController::processMouseRotation(float xoffset)
{
    if (!controlledCharacter_) return;
    
    // 更新角色的yaw角度（水平旋转）
    controlledCharacter_->yaw += xoffset;
    
    // 更新角色的front向量
    updateCharacterFront();
}

void PlayController::updateCharacterFront()
{
    if (!controlledCharacter_) return;
    
    // 根据yaw角度计算front向量（只在水平面旋转）
    float yawRad = glm::radians(controlledCharacter_->yaw);
    controlledCharacter_->front.x = cos(yawRad);
    controlledCharacter_->front.y = 0.0f;  // 保持水平
    controlledCharacter_->front.z = sin(yawRad);
    controlledCharacter_->front = glm::normalize(controlledCharacter_->front);
}

void PlayController::updateCameraToHead()
{
    if (!controlledCharacter_) return;

    // 使用 Character 内部的头骨信息
    glm::vec3 headPos = controlledCharacter_->headPosition + cameraOffset_;
    glm::vec3 headForward = controlledCharacter_->headForward;

    controlledCamera_.setPos(headPos);
    controlledCamera_.setFront(headForward);
}
