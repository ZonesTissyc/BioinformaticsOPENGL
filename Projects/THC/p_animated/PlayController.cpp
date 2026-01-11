#include <games/PlayController.h>
#include <games/character.h>
#include <glm/gtc/matrix_transform.hpp>

PlayController::PlayController(Character* character, float moveSpeed)
    : controlledCharacter_(character), moveSpeed_(moveSpeed) {
}

void PlayController::processInput(GLFWwindow* window, float deltaTime) {
    if (!controlledCharacter_ || !controlledCharacter_->alive)
        return;

    handleMovement(window, deltaTime);
    handleActions(window);
}

void PlayController::handleMovement(GLFWwindow* window, float deltaTime) {
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

void PlayController::handleActions(GLFWwindow* window) {
    // 攻击动作（一次性动画）
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Attack);
    }

    // 死亡动作（一次性动画）
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        controlledCharacter_->SetAction(Character::Action::Death);
        controlledCharacter_->alive = false;
    }
}
