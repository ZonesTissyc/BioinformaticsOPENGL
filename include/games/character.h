#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <custom/renderer.h> 
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <custom/ModelTrans.h>

class Character : public Object {
public:
    enum Action { Stay, Run, Attack };

    Character(ModelBase* modelPtr, Shader* shader = nullptr, glm::vec3 position = glm::vec3(0.0f))
        : Object(modelPtr, shader, position), action(Stay), speed(2.5f), alive(true)
    {
        pAnimModel = dynamic_cast<ModelAnimated*>(model);

        if (pAnimModel) {
            auto defaultAnim = pAnimModel->GetDefaultAnimation();
            if (defaultAnim) {
                m_Animator = std::make_unique<Animator>(defaultAnim.get());
            }
            else {
                m_Animator = std::make_unique<Animator>(nullptr);
            }
        }
    }

    void Update(float deltaTime) {
        if (m_Animator) {
            m_Animator->UpdateAnimation(deltaTime);
        }
    }

    void PlayAnimation(const std::string& animName) {
        if (!pAnimModel || !m_Animator) return;
        auto anim = pAnimModel->GetAnimation(animName);
        if (anim) {
            m_Animator->PlayAnimation(anim.get());
        }
    }

   
    // 依然接收 shader 参数，传递给 Renderer
    void Draw(Shader& shader) {
        if (!model) return;

        // 1. 准备模型矩阵
        glm::mat4 modelMat = glm::mat4(1.0f); // 显式初始化为单位矩阵
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::scale(modelMat, scale);
        // 如果有旋转：
        // modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(0,1,0));

        // 2. 准备骨骼数据
        std::vector<glm::mat4> bones; // 默认构造为空
        if (pAnimModel && m_Animator) {
            bones = m_Animator->GetFinalBoneMatrices();
        }

        // 3. 提交给 Renderer
        Renderer::Submit(shader, model, modelMat, bones);
    }

    Animator* GetAnimator() const { return m_Animator.get(); }

public:
    glm::vec3 front;
    Action action;
    float speed;
    bool alive;

private:
    ModelAnimated* pAnimModel = nullptr;
    std::unique_ptr<Animator> m_Animator;
};