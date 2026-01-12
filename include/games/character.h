#pragma once

#include <games/object.h>
#include <custom/model_animated.h>
#include <custom/animator.h>
#include <custom/renderer.h>
#include <custom/ModelTrans.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        
        // 初始化 front 向量，使其与 yaw 角度一致
        glm::vec3 initialFront;
        initialFront.x = cos(glm::radians(yaw));
        initialFront.y = 0.0f;
        initialFront.z = sin(glm::radians(yaw));
        front = glm::normalize(initialFront);
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

        // 使用 ModelTrans 来构建模型矩阵
        ModelTrans modelTrans;
        modelTrans.translate(position);
        modelTrans.rotate(yaw + 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));  // +90 度是因为模型初始朝向可能是 X 方向
        modelTrans.scale(scale);
        
        glm::mat4 modelMat = modelTrans.getModelMatrix();

        const auto& bones =
            (pAnimModel && m_Animator)
            ? m_Animator->GetFinalBoneMatrices()
            : std::vector<glm::mat4>();

        Renderer::Submit(shader, model, modelMat, bones);
    }

    Animator* GetAnimator() const { return m_Animator.get(); }

    // ============================
    // 处理鼠标旋转
    // ============================
    void ProcessMouseRotation(float xoffset, float yoffset, float sensitivity = 0.1f)
    {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;

        // 更新 front 方向向量（基于 yaw + 90.0f，与模型旋转保持一致）
        float adjustedYaw = yaw + 90.0f;  // 与 Draw() 和 UpdateHeadBoneInfo() 中的旋转保持一致
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(adjustedYaw));
        newFront.y = 0.0f;  // 保持水平旋转
        newFront.z = sin(glm::radians(adjustedYaw));
        front = glm::normalize(newFront);
    }

public:
    Action action;
    float speed{ 2.5f };
    bool alive{ true };
    float yaw{ -90.0f };  // 水平旋转角度（度），初始朝向 -Z 方向

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

        // 计算模型矩阵（与 Draw() 中一致，必须包含旋转！）
        ModelTrans modelTrans;
        modelTrans.translate(position);
        modelTrans.rotate(yaw + 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));  // 与 Draw() 中的旋转保持一致
        modelTrans.scale(scale);
        glm::mat4 modelMat = modelTrans.getModelMatrix();

        // 将骨骼矩阵转换为世界空间：模型矩阵 × 骨骼世界矩阵
        glm::mat4 worldHeadMat = modelMat * boneWorldMat;

        // 提取世界空间位置和方向
        headPosition = glm::vec3(worldHeadMat[3]); // 提取平移
        headForward = glm::normalize(glm::vec3(worldHeadMat * glm::vec4(0, 0.0f, 1.0f, 0))); // -Z 为前
        
        // 更新 front 为头骨的水平投影方向（确保与摄像机方向一致）
        glm::vec3 headForwardHorizontal = glm::vec3(headForward.x, 0.0f, headForward.z);
        float horizontalLen = glm::length(headForwardHorizontal);
        if (horizontalLen > 1e-4f) {
            front = glm::normalize(headForwardHorizontal);
        }
    }
};
