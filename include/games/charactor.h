#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>

// 所有人物的类，包括具有动画的玩家和敌人
class Charactor : public Object {
public:
    enum Action { Stay, Run, Attack };

    Charactor(ModelBase* modelPtr, Shader* shader = nullptr, glm::vec3 position = glm::vec3(0.0f))
        : Object(modelPtr, shader, position), action(Stay), speed(2.5f), alive(true)
    {
        // 尝试将 ModelBase 向下转型为 ModelAnimated
        // 如果是静态模型，pAnimModel 将为 nullptr
        pAnimModel = dynamic_cast<ModelAnimated*>(model);

        if (pAnimModel) {
            // 如果是动画模型，初始化 Animator
            // 默认播放第一个动画
            auto defaultAnim = pAnimModel->GetDefaultAnimation();
            if (defaultAnim) {
                m_Animator = std::make_unique<Animator>(defaultAnim.get());
            }
            else {
                // 如果没有动画资源，创建一个空的 Animator
                m_Animator = std::make_unique<Animator>(nullptr);
            }
        }
    }

    // 游戏循环中的更新逻辑
    void Update(float deltaTime) {
        if (m_Animator) {
            m_Animator->UpdateAnimation(deltaTime);
        }

        // 可以在这里根据 action 状态自动切换动画
        // 例如：if (action == Run) PlayAnimation("run");
    }

    // 播放指定名称的动画
    void PlayAnimation(const std::string& animName) {
        if (!pAnimModel || !m_Animator) return;

        auto anim = pAnimModel->GetAnimation(animName);
        if (anim) {
            m_Animator->PlayAnimation(anim.get());
        }
        else {
            std::cout << "Warning: Animation " << animName << " not found!" << std::endl;
        }
    }

    // 重写/新增绘制方法
    // 注意：Object 类中没有 virtual Draw，所以外部调用时需要通过 Charactor 指针调用，
    // 或者你在 Object 里加了 virtual Draw
    void Draw(const glm::vec3& camPos) {
        if (!shader) return;
        shader->use();

        // 设置通用的 Model 矩阵 (假设 shader 中叫 "model")
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::scale(modelMat, scale);
        // 这里应补充旋转逻辑...
        shader->setMat4("model", modelMat);

        if (pAnimModel && m_Animator) {
            // 1. 获取当前计算好的骨骼矩阵
            const auto& matrices = m_Animator->GetFinalBoneMatrices();
            // 2. 调用 ModelAnimated 特有的带矩阵的 Draw
            pAnimModel->Draw(*shader, matrices);
        }
        else {
            // 静态模型或无动画
            model->Draw(*shader, camPos);
        }
    }

    // 获取 Animator 指针（如果需要更细粒度控制）
    Animator* GetAnimator() const { return m_Animator.get(); }

public:
    glm::vec3 front;
    Action action;
    float speed;
    bool alive;

private:
    // 只有当模型确实是 ModelAnimated 时，这两个才有效
    ModelAnimated* pAnimModel = nullptr; // 弱引用，资源归 ResourceManager 或外部管理
    std::unique_ptr<Animator> m_Animator;
};