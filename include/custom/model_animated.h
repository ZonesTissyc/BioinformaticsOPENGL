#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <custom/model_base.h>
#include <custom/model_anim_data.h>
#include <custom/animation.h>
#include <custom/animator.h>
#include <custom/shader.h>

class ModelAnimated : public ModelBase
{
public:
    ModelAnimated(std::shared_ptr<ModelAnimData> data,
        std::shared_ptr<Animation> animation)
        : m_Data(std::move(data)),
        m_Animation(std::move(animation))
    {
        if (m_Animation)
            m_Animator = std::make_unique<Animator>(m_Animation.get());
    }

    void Update(float deltaTime) override
    {
        if (m_Animator)  // 注意这里使用指针
            m_Animator->UpdateAnimation(deltaTime);
    }

    void Draw(Shader& shader, const glm::vec3& /*camPos*/) override
    {
        if (!m_Data) return;

        // 安全地取得骨骼矩阵（避免临时引用问题）
        const std::vector<glm::mat4>* pFinal = nullptr;
        static const std::vector<glm::mat4> emptyBones;
        if (m_Animator)
            pFinal = &m_Animator->GetFinalBoneMatrices();
        else
            pFinal = &emptyBones;

        const auto& finalMatrices = *pFinal;
        // 注意：不要超过 shader 的 MAX_BONES
        size_t count = finalMatrices.size();
        for (size_t i = 0; i < count; ++i)
        {
            std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
            shader.setMat4(name.c_str(), finalMatrices[i]);
        }

        m_Data->DrawMeshes(shader);
    }


    bool IsAnimated() const override { return true; }

private:
    std::shared_ptr<ModelAnimData> m_Data;
    std::shared_ptr<Animation> m_Animation;
    std::unique_ptr<Animator> m_Animator;  // 用智能指针解决默认构造问题
};
