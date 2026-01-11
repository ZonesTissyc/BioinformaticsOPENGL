#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <custom/animation.h>
#include <custom/bone.h>

#include <algorithm>
#include <cmath>

class Animator
{
public:
    enum class AnimationPlayMode {
        Loop,
        Once
    };

    Animator(Animation* animation = nullptr)
    {
        m_CurrentTime = 0.0f;
        m_CurrentAnimation = animation;

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
        m_FinalBoneMatrices.reserve(finalSize);

        for (int i = 0; i < finalSize; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // ============================
    // 核心：更新动画（支持 Once）
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
        else // Once
        {
            if (m_CurrentTime >= duration)
            {
                m_CurrentTime = duration; // 停在最后一帧
                m_Finished = true;
            }
        }

        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(),
            glm::mat4(1.0f));
    }

    // ============================
    // 播放动画（防止每帧重播）
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
    // 骨骼计算（未改）
    // ============================
    void CalculateBoneTransform(const AssimpNodeData* node,
        glm::mat4 parentTransform)
    {
        if (!node || !m_CurrentAnimation)
            return;

        const std::string& nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_CurrentAnimation->FindBone(nodeName);
        if (bone)
        {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        auto it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end())
        {
            int index = it->second.id;
            const glm::mat4& offset = it->second.offset;
            if (index >= 0 && index < (int)m_FinalBoneMatrices.size())
                m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;

    Animation* m_CurrentAnimation{ nullptr };
    float m_CurrentTime{ 0.0f };

    AnimationPlayMode m_PlayMode{ AnimationPlayMode::Loop };
    bool m_Finished{ false };
};
