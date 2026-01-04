#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class Character {
public:
    enum Action { Stay, Run, Attack };

    Character(glm::vec3 pos = glm::vec3(0.0f))
        : position(pos), front(0.0f, 0.0f, -1.0f), action(Saty), speed(2.5f), alive(true) {
    }

    // ---------- 属性 ----------
    glm::vec3 position;
    glm::vec3 front;
    float speed;
    bool alive;
    Action action;

    // ---------- 动作 ----   ------
    void moveForward(float dt) {
        position += front * speed * dt;
        action = Run;
    }

    void moveBackward(float dt) {
        position -= front * speed * dt;
        action = Run;
    }

    void stop() {
        action = Idle;
    }

    // ---------- 射线接口 ----------
    struct Ray {
        glm::vec3 origin;
        glm::vec3 dir;
    };

    Ray shoot() const {
        return { position, front };
    }

    // ---------- 受伤 ----------
    void takeDamage(float dmg) {
        alive = false; // 简单处理
        std::cout << "Character at " << position.x << "," << position.y << "," << position.z << " is dead\n";
    }
};
