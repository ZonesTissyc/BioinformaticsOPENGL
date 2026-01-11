#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <custom/renderer.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include <string>

class Character : public Object {
public:
    // 动作枚举
    enum class Action { Stay, Run, Attack , Death};

    Character(ModelBase* modelPtr, Shader* shader = nullptr,
        glm::vec3 position = glm::vec3(0.0f))
        : Object(modelPtr, shader, position)
    {
        pAnimModel = dynamic_cast<ModelAnimated*>(model);

        if (pAnimModel)
        {
            auto defaultAnim = pAnimModel->GetDefaultAnimation();
            m_Animator = std::make_unique<Animator>(
                defaultAnim ? defaultAnim.get() : nullptr);
        }

        // 初始化 Action → Animation 名称映射
        m_ActionToAnim = {
    { Action::Stay,  "idleWoutGun" }, // idleWithoutGun
    { Action::Run,   "run" },         // run
    { Action::Attack,"shooting" },
    { Action::Death, "death" },
	    };

        // 默认状态
        SetAction(Action::Stay);
    }

    // ============================
    // 状态机 Update
    // ============================
    void Update(float deltaTime)
    {
        if (!m_Animator) return;

        // 推进动画时间
        m_Animator->UpdateAnimation(deltaTime);

        // 一次性动画播完后自动回 Idle
        if (action == Action::Attack && m_Animator->IsFinished())
        {
            SetAction(Action::Stay);
        }
    }

    // ============================
    // 切换状态（统一管理动画播放）
    // ============================
    void SetAction(Action newAction)
    {
        if (action == newAction)
            return; // 状态未变，不重复播放

        action = newAction;

        bool once = (action == Action::Attack); // 攻击动作只播放一次

        auto it = m_ActionToAnim.find(action);
        if (it != m_ActionToAnim.end())
        {
            PlayAnimation(it->second, once);
        }
    }

    // ============================
    // 播放动画（支持 Once / Loop）
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

    // ============================
    // 绘制模型
    // ============================
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

    std::map<Action, std::string> m_ActionToAnim;
};
