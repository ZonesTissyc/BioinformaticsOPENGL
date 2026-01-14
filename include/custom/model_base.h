#pragma once

#include <custom/shader.h>
#include <glm/glm.hpp>

/*
静态与动态模型的基类
这里高度抽象
 */
class ModelBase
{
public:
    virtual ~ModelBase() = default;

    
    virtual void Draw(Shader& shader, const glm::vec3& camPos) = 0;


    virtual void Update(float deltaTime)
    {
        // default: do nothing
    }

    virtual bool IsAnimated() const
    {
        return false;
    }


};

