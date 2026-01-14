#pragma once

#include <games/enemy.h>
#include <custom/Camera.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

/*
CombatSystem 类：管理射击系统和敌人命中检测
命中判断逻辑，现在用最简单的射线-球体模型
*/
class CombatSystem {
public:
    CombatSystem(Camera& camera, float maxShootDistance = 100.0f, bool enableDebug = true)
        : camera_(camera), maxShootDistance_(maxShootDistance), enableDebug_(enableDebug)
    {
    }


    
    // 添加敌人到系统
    void AddEnemy(Enemy* enemy)
    {
        if (enemy)
        {
            enemies_.push_back(enemy);
            if (enableDebug_)
            {
                std::cout << "Enemy added to combat system, position: (" 
                          << enemy->position.x << ", " << enemy->position.y << ", " << enemy->position.z << ")" << std::endl;
            }
        }
    }
    
    // 移除敌人
    void RemoveEnemy(Enemy* enemy)
    {
        enemies_.erase(
            std::remove_if(enemies_.begin(), enemies_.end(),
                [enemy](Enemy* e) { return e == enemy; }),
            enemies_.end()
        );
    }
    
    // 清空所有敌人
    void ClearEnemies()
    {
        enemies_.clear();
    }
    
    // 获取敌人列表
    const std::vector<Enemy*>& GetEnemies() const
    {
        return enemies_;
    }

    // 是否成功射击
    bool ProcessShootInput(GLFWwindow* window)
    {
        bool leftMousePressedNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        
        // 检测鼠标左键按下
        if (leftMousePressedNow && !leftMousePressedLast_)
        {
            leftMousePressedLast_ = leftMousePressedNow;
            Shoot();
            return true;
        }
        
        leftMousePressedLast_ = leftMousePressedNow;
        return false;
    }
    
    // 执行射击（发射射线并检测命中）
    void Shoot()
    {
        // 获取射线起点和方向（从摄像机）
        glm::vec3 rayOrigin = camera_.getPos();
        glm::vec3 rayDirection = glm::normalize(camera_.getFront());
        
        if (enableDebug_)
        {
            std::cout << "=== Shoot ===" << std::endl;
            std::cout << "Ray origin: (" << rayOrigin.x << ", " << rayOrigin.y << ", " << rayOrigin.z << ")" << std::endl;
            std::cout << "Ray direction: (" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << ")" << std::endl;
        }
        
        // 检测所有敌人
        bool hitAny = false;
        for (Enemy* enemy : enemies_)
        {
            if (!enemy || enemy->IsDead())
                continue;
            
            glm::vec3 hitPoint;
            if (enemy->CheckRayHitAndKill(rayOrigin, rayDirection, hitPoint, maxShootDistance_))
            {
                hitAny = true;
                OnEnemyHit(enemy, hitPoint);
            }
            else if (enableDebug_)
            {
                // 输出未命中的调试信息
                std::cout << "Enemy position: (" << enemy->position.x << ", " << enemy->position.y << ", " << enemy->position.z << ")" << std::endl;
                std::cout << "Enemy hit center: (" << enemy->GetHitCenter().x << ", " 
                          << enemy->GetHitCenter().y << ", " << enemy->GetHitCenter().z << ")" << std::endl;
                std::cout << "Enemy hit radius: " << enemy->GetHitRadius() << std::endl;
            }
        }
        
        if (!hitAny && enableDebug_)
        {
            std::cout << "No enemy hit" << std::endl;
        }
    }
    

    // 设置最大射击距离
    void SetMaxShootDistance(float distance)
    {
        maxShootDistance_ = distance;
    }
    
    // 获取最大射击距离
    float GetMaxShootDistance() const
    {
        return maxShootDistance_;
    }
    
    // 启用/禁用调试信息
    void SetDebugEnabled(bool enabled)
    {
        enableDebug_ = enabled;
    }
    
    // 是否启用调试信息
    bool IsDebugEnabled() const
    {
        return enableDebug_;
    }

private:

    // 当敌人被命中时调用
    void OnEnemyHit(Enemy* enemy, const glm::vec3& hitPoint)
    {
        if (enableDebug_)
        {
            std::cout << "*** Enemy hit! Hit point: (" 
                      << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << ")" << std::endl;
            std::cout << "Enemy position: (" << enemy->position.x << ", " 
                      << enemy->position.y << ", " << enemy->position.z << ")" << std::endl;
            std::cout << "Enemy killed" << std::endl;
        }
        
        // 这里可以添加其他逻辑，以后有时间再说
    }

private:
    Camera& camera_;                           // 摄像机引用
    std::vector<Enemy*> enemies_;              // 敌人列表
    float maxShootDistance_;                   // 最大射击距离
    bool enableDebug_;                         // 是否启用调试信息
    bool leftMousePressedLast_ = false;        // 上一帧鼠标左键状态
};

