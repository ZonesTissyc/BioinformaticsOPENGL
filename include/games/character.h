#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <custom/renderer.h>
#include <glm/glm.hpp>
#include <memory>

class Character : public Object {
public:
    enum Action { Stay, Run, Attack };

    Character(ModelBase* modelPtr, Shader* shader = nullptr,
        glm::vec3 position = glm::vec3(0.0f))
        : Object(modelPtr, shader, position), action(Stay)
    {
        pAnimModel = dynamic_cast<ModelAnimated*>(model);

        if (pAnimModel)
        {
            auto defaultAnim = pAnimModel->GetDefaultAnimation();
            m_Animator = std::make_unique<Animator>(
                defaultAnim ? defaultAnim.get() : nullptr);
        }
    }

    void Update(float deltaTime)
    {
        if (!m_Animator) return;

        m_Animator->UpdateAnimation(deltaTime);

        // 可选：一次性动画播完自动回 Idle
        if (action == Attack && m_Animator->IsFinished())
        {
            action = Stay;
            PlayAnimation("idleWithoutGun");
        }
    }

    // ============================
    // 播放动画（支持 Once）
    // ============================
    void PlayAnimation(const std::string& animName, bool once = false)
    {
        if (!pAnimModel || !m_Animator) return;

        auto anim = pAnimModel->GetAnimation(animName);
        if (!anim) return;

        m_Animator->PlayAnimation(
            anim.get(),
            once ? Animator::AnimationPlayMode::Once
            : Animator::AnimationPlayMode::Loop
        );
    }

    void Draw(Shader& shader)
    {
        if (!model) return;

        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), position);
        modelMat = glm::scale(modelMat, scale);

        const auto& bones =
            (pAnimModel && m_Animator)
            ? m_Animator->GetFinalBoneMatrices()
            : std::vector<glm::mat4>();

        Renderer::Submit(shader, model, modelMat, bones);
    }

    Animator* GetAnimator() const { return m_Animator.get(); }

public:
    Action action;
    float speed{ 2.5f };
    bool alive{ true };

private:
    ModelAnimated* pAnimModel{ nullptr };
    std::unique_ptr<Animator> m_Animator;
};
