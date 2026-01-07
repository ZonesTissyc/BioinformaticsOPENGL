#pragma once

#include <games/object.h>
#include <custom/model_base.h>
#include <glm/glm.hpp>
#include <iostream>

// 所有人物的类，包括具有动画的玩家和敌人

class Charactor : public Object {
public:
    enum Action { Stay, Run, Attack };

    Charactor(ModelBase* model, Shader* shader = nullptr, glm::vec3 position = glm::vec3(0.0f))
        : Object(model, shader, position), action(Stay), speed(2.5f), alive(true) {
    }

    // 
    glm::vec3 front;
    Action action;
    float speed;
    bool alive;
};