#include <games/PlayController.h>
#include <games/character.h>
#include <custom/Camera.h>
#include <glm/gtc/matrix_transform.hpp>

PlayController::PlayController(Character* character, Camera& camera, float moveSpeed)
    : controlledCharacter_(character), controlledCamera_(camera), moveSpeed_(moveSpeed)
{
}

void PlayController::processInput(GLFWwindow* window, float deltaTime)
{
    if (!controlledCharacter_ || !controlledCharacter_->alive)
        return;

    handleMovement(window, deltaTime);
    handleActions(window);
    updateCameraToHead(); // 每帧同步摄像机
}

void PlayController::handleMovement(GLFWwindow* window, float deltaTime)
{
    glm::vec3 direction(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) direction.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) direction.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) direction.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) direction.x -= 1.0f;

    if (glm::length(direction) > 0.0f) {
        direction = glm::normalize(direction);
        
        // 基于角色的 yaw 角度计算移动方向
        float yawRad = glm::radians(controlledCharacter_->yaw);
        glm::vec3 forward = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));
        glm::vec3 right = glm::vec3(-sin(yawRad), 0.0f, cos(yawRad));
        
        // 计算世界空间移动方向
        glm::vec3 moveDir = forward * direction.z + right * direction.x;
        controlledCharacter_->position += moveDir * moveSpeed_ * deltaTime;

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

void PlayController::processMouseInput(float xoffset, float yoffset, float sensitivity)
{
    if (!controlledCharacter_ || !controlledCharacter_->alive)
        return;
    
    controlledCharacter_->ProcessMouseRotation(xoffset, yoffset, sensitivity);
}

void PlayController::updateCameraToHead()
{
    if (!controlledCharacter_) return;

    // 使用 Character 内部的头骨信息
    glm::vec3 headPos = controlledCharacter_->headPosition;
    glm::vec3 headForward = controlledCharacter_->headForward;

    // 检查头方向是否有效（长度不能太小）
    float headForwardLen = glm::length(headForward);
    if (headForwardLen < 1e-4f)
        return;
    
    headForward = glm::normalize(headForward);

    // 期望：摄像机始终在头骨“后面一点”，并朝向头骨前方
    // cameraOffset_.z 用作“往后拉多远”（第三人称距离，正值表示离角色更远）
    // cameraOffset_.y 用作“抬高多少”
    float backDist = cameraOffset_.z;
    float upOffset = cameraOffset_.y;

    glm::vec3 camPos =
        headPos
        - headForward * backDist          // 往头骨反方向（后方）拉开距离
        + glm::vec3(0.0f, upOffset, 0.0f); // 稍微抬高一点

    controlledCamera_.setPos(camPos);
    controlledCamera_.setFront(headForward);  // 始终朝向头骨前方
}
