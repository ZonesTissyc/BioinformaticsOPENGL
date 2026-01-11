#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Character;

class PlayController {
public:
    PlayController(Character* character, float moveSpeed = 2.5f);

    void processInput(GLFWwindow* window, float deltaTime);

private:
    Character* controlledCharacter_ = nullptr;
    float moveSpeed_;

    // 内部方法
    void handleMovement(GLFWwindow* window, float deltaTime);
    void handleActions(GLFWwindow* window);
};

