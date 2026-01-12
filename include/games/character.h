#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <custom/renderer.h>
#include <custom/ModelTrans.h>
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

        // 初始化头骨信息
        headPosition = glm::vec3(0.0f);
        headForward = glm::vec3(0.0f, 0.0f, -1.0f);
        
        // 初始化朝向（-90度对应-Z方向）
        yaw = -90.0f;
        front = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    // ============================
    // 状态机 Update
    // ============================
    void Update(float deltaTime)
    {
        if (!m_Animator) return;

        // 推进动画时间
        m_Animator->UpdateAnimation(deltaTime);

        // 更新头骨信息（每帧）
        UpdateHeadBoneInfo();



        // 一次性动画播完后自动回 Idle
        if (action == Action::Attack && m_Animator->IsFinished())
        {
            SetAction(Action::Stay);
        }
    }

    // ============================
    // 切换状态（统一管理动画播放）
    // ============================
    void SetAction(Action newAction, bool once = false)
    {
        if (action == newAction)
            return; // 状态未变，不重复播放

        action = newAction;

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

        // 使用 ModelTrans 构建变换矩阵
        ModelTrans trans;
        trans.translate(position);
        trans.rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));  // 绕Y轴旋转（水平旋转）
        trans.scale(scale);
        
        glm::mat4 modelMat = trans.getModelMatrix();

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
    float yaw{ -90.0f };  // 水平旋转角度（度），-90度表示初始朝向-Z方向

    // 头骨成员变量（每帧更新）
   // ============================
    glm::vec3 headPosition;  // 世界空间位置
    glm::vec3 headForward;   // 世界空间前方向

private:
    ModelAnimated* pAnimModel{ nullptr };
    std::unique_ptr<Animator> m_Animator;

    std::map<Action, std::string> m_ActionToAnim;

    // 每帧更新头骨信息
  // ============================
    void UpdateHeadBoneInfo()
    {
        if (!m_Animator) return;

        constexpr int HEAD_BONE_INDEX = 15; // 你的头骨ID
        
        // 获取骨骼世界矩阵（相对于模型根节点）
        glm::mat4 boneWorldMat;
        if (!m_Animator->GetBoneWorldMatrix(HEAD_BONE_INDEX, boneWorldMat))
            return;

        // 计算模型矩阵（与 Draw() 中一致，用于转换到世界空间）
        ModelTrans trans;
        trans.translate(position);
        trans.rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));  // 绕Y轴旋转
        trans.scale(scale);
        glm::mat4 modelMat = trans.getModelMatrix();

        // 将骨骼矩阵转换为世界空间：模型矩阵 × 骨骼世界矩阵
        glm::mat4 worldHeadMat = modelMat * boneWorldMat;

        // 提取世界空间位置和方向
        headPosition = glm::vec3(worldHeadMat[3]); // 提取平移
        headForward = glm::normalize(glm::vec3(worldHeadMat * glm::vec4(0, 0, -1, 0))); // -Z 为前
    }
};
