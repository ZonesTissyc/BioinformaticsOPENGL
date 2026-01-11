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
        controlledCharacter_->position += direction * moveSpeed_ * deltaTime;

        // 切换 Run 动画
        controlledCharacter_->SetAction(Character::Action::Run);
        // 朝向移动方向
        controlledCharacter_->front = glm::normalize(direction);
    }
    else {
        // 切换 Idle 动画
        controlledCharacter_->SetAction(Character::Action::Stay);
    }
}

void PlayController::handleActions(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Attack, true);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Death, true);
        controlledCharacter_->alive = false;
    }
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
