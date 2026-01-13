#pragma once

#include <games/character.h>
#include <glm/glm.hpp>

/*
Enemy 类继承自 Character，用于表示游戏中的敌人
添加了命中检测功能，支持射线/子弹命中判断
*/
class Enemy : public Character {
public:
    Enemy(ModelBase* modelPtr, Shader* shader = nullptr,
        glm::vec3 position = glm::vec3(0.0f),
        glm::vec3 hitCenter = glm::vec3(0.0f),
        float hitRadius = 0.5f)
        : Character(modelPtr, shader, position),
          hitCenter_(hitCenter),
          hitRadius_(hitRadius),
          isDead_(false)
    {
    }

    // ============================
    // 命中检测相关方法
    // ============================
    
    // 获取命中中心点（世界空间）
    glm::vec3 GetHitCenter() const
    {
        // 命中中心 = 角色位置 + 相对偏移
        return position + hitCenter_;
    }
    
    // 获取命中半径
    float GetHitRadius() const
    {
        return hitRadius_;
    }
    
    // 设置命中中心点（相对位置）
    void SetHitCenter(glm::vec3 hitCenter)
    {
        hitCenter_ = hitCenter;
    }
    
    // 设置命中半径
    void SetHitRadius(float radius)
    {
        hitRadius_ = radius;
    }
    
    // ============================
    // 射线命中检测
    // ============================
    // 检测射线是否命中敌人（使用球体碰撞检测）
    // rayOrigin: 射线起点（世界空间）
    // rayDirection: 射线方向（已归一化）
    // maxDistance: 射线最大距离（可选，用于限制检测范围）
    // 返回：是否命中，如果命中，outHitPoint 返回命中点位置
    bool CheckRayHit(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                     glm::vec3& outHitPoint, float maxDistance = 1000.0f) const
    {
        glm::vec3 center = GetHitCenter();
        
        // 计算从射线起点到球心的向量
        glm::vec3 oc = center - rayOrigin;
        
        // 计算射线方向上的投影长度
        float projection = glm::dot(oc, rayDirection);
        
        // 如果投影为负，说明球在射线后方，未命中
        if (projection < 0.0f)
            return false;
        
        // 如果投影超过最大距离，未命中
        if (projection > maxDistance)
            return false;
        
        // 计算射线起点到球心的最近点
        glm::vec3 closestPoint = rayOrigin + rayDirection * projection;
        
        // 计算最近点到球心的距离
        float distanceToCenter = glm::length(closestPoint - center);
        
        // 如果距离小于半径，则命中
        if (distanceToCenter <= hitRadius_)
        {
            // 计算命中点（球面上的点）
            glm::vec3 toHitPoint = glm::normalize(closestPoint - center);
            outHitPoint = center + toHitPoint * hitRadius_;
            return true;
        }
        
        return false;
    }
    
    // 简化版本：只返回是否命中，不返回命中点
    bool CheckRayHit(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                     float maxDistance = 1000.0f) const
    {
        glm::vec3 dummyHitPoint;
        return CheckRayHit(rayOrigin, rayDirection, dummyHitPoint, maxDistance);
    }
    
    // ============================
    // 点命中检测（用于子弹等）
    // ============================
    // 检测一个点是否在命中范围内
    bool CheckPointHit(const glm::vec3& point) const
    {
        glm::vec3 center = GetHitCenter();
        float distance = glm::length(point - center);
        return distance <= hitRadius_;
    }
    
    // ============================
    // 命中处理（触发死亡）
    // ============================
    // 当敌人被命中时调用此方法，会触发死亡并播放 Death 动画
    // 返回：是否成功触发死亡（如果已经死亡则返回 false）
    bool OnHit()
    {
        // 如果已经死亡，不再处理
        if (isDead_ || !alive)
            return false;
        
        // 触发死亡
        isDead_ = true;
        alive = false;
        
        // 播放 Death 动画（Once 模式，播放一次）
        SetAction(Action::Death, true);
        
        return true;
    }
    
    // ============================
    // 状态查询
    // ============================
    bool IsDead() const
    {
        return isDead_ || !alive;
    }
    
    // ============================
    // 带死亡检测的命中方法
    // ============================
    // 检测射线命中，如果命中则自动触发死亡
    bool CheckRayHitAndKill(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                            glm::vec3& outHitPoint, float maxDistance = 1000.0f)
    {
        if (IsDead())
            return false;
            
        if (CheckRayHit(rayOrigin, rayDirection, outHitPoint, maxDistance))
        {
            OnHit();
            return true;
        }
        return false;
    }
    
    // 检测点命中，如果命中则自动触发死亡
    bool CheckPointHitAndKill(const glm::vec3& point)
    {
        if (IsDead())
            return false;
            
        if (CheckPointHit(point))
        {
            OnHit();
            return true;
        }
        return false;
    }

private:
    glm::vec3 hitCenter_;   // 命中中心点（相对于角色位置的偏移）
    float hitRadius_;       // 命中半径
    bool isDead_;           // 是否已死亡（防止重复触发）
};

