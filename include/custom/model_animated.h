#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>

#include <custom/model_base.h>
#include <custom/model_anim_data.h>
#include <custom/animation.h>
#include <custom/shader.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

class ModelAnimated  : public ModelBase
{
public:

    ModelAnimated(std::shared_ptr<ModelAnimData> data,
        std::shared_ptr<Animation> animation)
        : m_Data(std::move(data))
    {
        if (animation)
        {
            AddAnimation("default", animation);
        }
    }

    ModelAnimated(std::shared_ptr<ModelAnimData> data,
        const std::map<std::string, std::shared_ptr<Animation>>& animations)
        : m_Data(std::move(data)), m_Animations(animations)
    {
    }

    // 资源管理方法
    void AddAnimation(const std::string& name, std::shared_ptr<Animation> animation)
    {
        m_Animations[name] = std::move(animation);
    }

    std::shared_ptr<Animation> GetAnimation(const std::string& name) const
    {
        auto it = m_Animations.find(name);
        return (it != m_Animations.end()) ? it->second : nullptr;
    }

    // 获取所有动画（方便调试或UI显示）
    const std::map<std::string, std::shared_ptr<Animation>>& GetAllAnimations() const
    {
        return m_Animations;
    }

    // 获取第一个/默认动画，用于初始化
    std::shared_ptr<Animation> GetDefaultAnimation() const {
        if (m_Animations.empty()) return nullptr;
        return m_Animations.begin()->second;
    }

    // --------------------------------------------------------
    // Draw 方法重构
    // --------------------------------------------------------

    // 1. 继承自 ModelBase 的接口
    // 如果直接调用这个，说明没有传入骨骼矩阵，默认绘制绑定姿态（单位矩阵）
    void Update(float deltaTime) override
    {
        // 现在的 ModelAnimated 是无状态的资源，不需要 Update
    }

    void Draw(Shader& shader, const glm::vec3& /*camPos*/) override
    {
        if (!m_Data) return;

        // 默认填充单位矩阵，防止 shader 报错或模型变形
        static const std::vector<glm::mat4> identityBones(100, glm::mat4(1.0f));

        for (size_t i = 0; i < identityBones.size(); ++i)
        {
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", identityBones[i]);
        }
        m_Data->DrawMeshes(shader);
    }

    // 2. 新增：接收外部计算好的骨骼矩阵进行绘制
    void Draw(Shader& shader, const std::vector<glm::mat4>& finalBoneMatrices)
    {
        if (!m_Data) return;

        size_t count = finalBoneMatrices.size();
        for (size_t i = 0; i < count; ++i)
        {
            // 注意：这里建议优化，使用 Uniform Buffer Object (UBO) 或 Texture Buffer 传递骨骼矩阵性能更好
            // 但为了保持当前代码风格，继续使用 setMat4
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
        }

        m_Data->DrawMeshes(shader);
    }

    bool IsAnimated() const override { return true; }

    // 静态加载函数保持不变
    static std::shared_ptr<ModelAnimated> LoadModelWithAllAnimations(
        const std::string& modelPath,
        bool gamma = false)
    {
        auto data = std::make_shared<ModelAnimData>(modelPath, gamma);
        std::map<std::string, std::shared_ptr<Animation>> animations;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate);



        if (scene && scene->mRootNode && scene->mNumAnimations > 0)
        {
            std::cout << "=== Loading Animations ===" << std::endl;
            std::cout << "Model: " << modelPath << std::endl;
            std::cout << "Found " << scene->mNumAnimations << " animation(s):" << std::endl;
            
            for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
            {
                std::string animName = scene->mAnimations[i]->mName.C_Str();
                if (animName.empty()) animName = "animation_" + std::to_string(i);

                std::cout << "  [" << i << "] " << animName << std::endl;

                auto animation = std::make_shared<Animation>(modelPath, data.get(), i);
                animations[animName] = animation;
            }
            std::cout << "===========================" << std::endl;
        }
        else
        {
            std::cout << "Warning: No animations found in " << modelPath << std::endl;
        }
        

        return std::make_shared<ModelAnimated>(data, animations);
    }

private:
    std::shared_ptr<ModelAnimData> m_Data;
    std::map<std::string, std::shared_ptr<Animation>> m_Animations;
    // 移除了 m_Animator, m_CurrentAnimationName
};

static aiNode* FindNode(aiNode* node, const std::string& name)
{
    if (!node) return nullptr;

    if (node->mName.C_Str() == name)
        return node;

    for (unsigned i = 0; i < node->mNumChildren; ++i)
    {
        aiNode* res = FindNode(node->mChildren[i], name);
        if (res) return res;
    }
    return nullptr;
}
