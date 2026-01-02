#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "animation.h"
#include <learnopengl//bone.h> // 确保引用的是本地的 bone.h

class Animator
{
public:
    // 构造函数：初始化时可以立即绑定一个动画
    Animator(Animation* animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        // 预留空间，避免频繁重新分配内存
        // 虽然这里预设了100，但在计算时我们会根据需要自动扩容
        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // 在每一帧渲染循环中调用
    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            // 1. 推进时间：当前时间 += 速度 * 时间增量
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

            // 2. 循环播放：如果时间超过动画时长，取余回到开始
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

            // 3. 核心计算：从根节点开始递归计算矩阵
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    // 切换动画
    void PlayAnimation(Animation* pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    // 递归函数：计算骨骼变换矩阵
  // 递归函数：计算骨骼变换矩阵
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        // 1. 查找当前节点是否有动画数据
        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;

            // ========================================================
            // 【关键修复】安全熔断机制
            // ========================================================
            // 设定一个合理的上限（例如 200）。如果 ID 大于这个数，绝对是脏数据。
            const int MAX_SAFE_BONE_ID = 200;

            if (index < 0) {
                // 忽略无效 ID
            }
            else if (index > MAX_SAFE_BONE_ID) {
                // 检测到异常 ID，直接跳过，防止 resize 导致内存爆炸卡死
                // 可以在这里打印一次警告（加个static bool防止刷屏）
                static bool hasPrintedError = false;
                if (!hasPrintedError) {
                    std::cout << "[CRITICAL WARNING] Ignored huge bone ID: " << index
                        << " on node: " << nodeName << ". This prevents freezing." << std::endl;
                    hasPrintedError = true;
                }
            }
            else {
                // ID 正常，进行扩容和赋值
                if (index >= m_FinalBoneMatrices.size()) {
                    m_FinalBoneMatrices.resize(index + 1, glm::mat4(1.0f));
                }
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }
            // ========================================================
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }
    // 获取计算好的矩阵数组（传给 Shader）
    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
};

#endif