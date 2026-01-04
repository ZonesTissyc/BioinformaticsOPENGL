#pragma once

#include <custom/shader.h>
#include <glm/glm.hpp>

/*
 * ModelBase
 * ----------
 * 所有模型（静态 / 动画）的统一抽象接口
 *
 * 设计原则：
 *  - Scene / Entity / Renderer 只认识 ModelBase
 *  - 不暴露 Mesh / Assimp / Bone / Animator
 *  - 类型差异只存在于派生类中
 */
class ModelBase
{
public:
    virtual ~ModelBase() = default;

    /*
     * Draw
     * ----
     * 每一帧渲染调用
     * 静态模型：直接 draw
     * 动画模型：在内部上传骨骼矩阵后 draw
     */
    virtual void Draw(Shader& shader, const glm::vec3& camPos) = 0;

    /*
     * Update
     * ------
     * 每帧更新（delta time）
     * 静态模型：默认空实现
     * 动画模型：更新 Animator
     */
    virtual void Update(float deltaTime)
    {
        // default: do nothing
    }

    /*
     * IsAnimated
     * ----------
     * 可选接口：用于调试 / 断言 / shader 选择
     * 正式渲染流程中一般不需要
     */
    virtual bool IsAnimated() const
    {
        return false;
    }


};

