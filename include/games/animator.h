#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <games/animation.h>
#include <custom/bone.h>

#include <algorithm>
#include <cmath>

class Animator
{
public:
    Animator(Animation* animation = nullptr)
    {
        m_CurrentTime = 0.0f;
        m_CurrentAnimation = animation;

        int maxBoneID = -1;
        if (animation) // 检查 animation 是否为空
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

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = std::fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation* pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        if (!node) return;
        if (!m_CurrentAnimation) {
            // 如果没有动画，仍需递归子节点以保持默认变换？通常直接 return。
            for (int i = 0; i < node->childrenCount; ++i)
                CalculateBoneTransform(&node->children[i], parentTransform * node->transformation);
            return;
        }

        const std::string& nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* BonePtr = m_CurrentAnimation->FindBone(nodeName);
        if (BonePtr)
        {
            BonePtr->Update(m_CurrentTime);
            nodeTransform = BonePtr->GetLocalTransform();
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


    // 返回 const 引用，避免每帧拷贝
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation{ nullptr };
    float m_CurrentTime{ 0.0f };
    float m_DeltaTime{ 0.0f };
};
