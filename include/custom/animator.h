#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>

#include <custom/animation.h>
#include <custom/bone.h>

class Animator
{
public:
    enum class AnimationPlayMode {
        Loop,
        Once
    };

    Animator(Animation* animation = nullptr)
        : m_CurrentAnimation(animation)
    {
        int maxBoneID = -1;
        if (animation)
        {
            auto& boneMap = animation->GetBoneIDMap();
            for (auto& item : boneMap)
            {
                if (item.second.id > maxBoneID)
                    maxBoneID = item.second.id;
            }
        }

        int finalSize = std::max(100, maxBoneID + 1);

        m_FinalBoneMatrices.resize(finalSize, glm::mat4(1.0f));
        m_BoneWorldMatrices.resize(finalSize, glm::mat4(1.0f));
    }

    // ============================
    // 更新动画
    // ============================
    void UpdateAnimation(float dt)
    {
        if (!m_CurrentAnimation || m_Finished)
            return;

        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

        float duration = m_CurrentAnimation->GetDuration();

        if (m_PlayMode == AnimationPlayMode::Loop)
        {
            m_CurrentTime = std::fmod(m_CurrentTime, duration);
        }
        else
        {
            if (m_CurrentTime >= duration)
            {
                m_CurrentTime = duration;
                m_Finished = true;
            }
        }

        CalculateBoneTransform(
            &m_CurrentAnimation->GetRootNode(),
            glm::mat4(1.0f)
        );
    }

    // ============================
    // 播放动画
    // ============================
    void PlayAnimation(Animation* pAnimation,
        AnimationPlayMode mode = AnimationPlayMode::Loop)
    {
        if (!pAnimation)
            return;

        if (m_CurrentAnimation == pAnimation && !m_Finished)
            return;

        m_CurrentAnimation = pAnimation;
        m_PlayMode = mode;
        m_CurrentTime = 0.0f;
        m_Finished = false;
    }

    bool IsFinished() const { return m_Finished; }

    // ============================
    // 关键：骨骼递归
    // ============================
    void CalculateBoneTransform(
        const AssimpNodeData* node,
        const glm::mat4& parentTransform)
    {
        if (!node || !m_CurrentAnimation)
            return;

        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_CurrentAnimation->FindBone(node->name);
        if (bone)
        {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransform = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        auto it = boneInfoMap.find(node->name);
        if (it != boneInfoMap.end())
        {
            int index = it->second.id;

            if (index >= 0 && index < (int)m_FinalBoneMatrices.size())
            {
                // ✅ GPU 蒙皮用（原逻辑，保留）
                m_FinalBoneMatrices[index] =
                    globalTransform * it->second.offset;

                // ✅ 新增：纯骨骼世界矩阵（给摄像机 / 逻辑用）
                m_BoneWorldMatrices[index] = globalTransform;
            }
        }

        for (int i = 0; i < node->childrenCount; i++)
        {
            CalculateBoneTransform(
                &node->children[i],
                globalTransform
            );
        }
    }

    // ============================
    // 原接口（给 Shader）
    // ============================
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return m_FinalBoneMatrices;
    }

    // ============================
    // ✅ 新接口（你现在最需要的）
    // ============================
    bool GetBoneWorldMatrix(int boneIndex, glm::mat4& out) const
    {
        if (boneIndex < 0 || boneIndex >= (int)m_BoneWorldMatrices.size())
            return false;

        out = m_BoneWorldMatrices[boneIndex];
        return true;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;   // GPU 蒙皮
    std::vector<glm::mat4> m_BoneWorldMatrices;   // 逻辑 / 摄像机

    Animation* m_CurrentAnimation{ nullptr };
    float m_CurrentTime{ 0.0f };

    AnimationPlayMode m_PlayMode{ AnimationPlayMode::Loop };
    bool m_Finished{ false };
};
