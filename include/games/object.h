#pragma once

#include <custom/model_base.h>
#include <glm/glm.hpp>

/*
这个是游戏中的实体对象的基类，保存一个模型指针
这个类的功能：保存模型的指针，表明这个实体对象的属性（位置，朝向，上方向等）
因为是基类，所以属性和方法都是公共的，其他实体对象可以继承这个类
属性就只要模型指针，位置，模型缩放大小，朝向，上方向即可
同时，我设置了要用哪个shader来渲染这个模型。（当然，也可以在渲染时指定），这里只记录
*/

class Object {
public:
    ModelBase* model;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
	glm::vec3 scale;
    Shader* shader;
    
    // 构造函数
    Object(ModelBase* model, Shader* shader = nullptr, glm::vec3 position = glm::vec3(0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
        : model(model), position(position), front(front), up(up), scale(1.0f), shader(shader) {
    }

    // 获取模型指针
    ModelBase* getModel() const {
        return model;
    }

    // 获取位置
    glm::vec3 getPosition() const {
        return position;
    }

    // 获取朝向
    glm::vec3 getFront() const {
        return front;
    }

    // 获取上方向
    glm::vec3 getUp() const {
        return up;
    }

    // 获取模型缩放大小
    glm::vec3 getScale() const {
        return scale;
    }

    // setter方法
    void setPosition(glm::vec3 position) {
        this->position = position;
    }

    // 设置朝向
    void setFront(glm::vec3 front) {
        this->front = front;
    }

    // 设置上方向
    void setUp(glm::vec3 up) {
        this->up = up;
    }
    // 设置模型缩放大小
    void setScale(glm::vec3 scale) {
        this->scale = scale;
    }

    // 是否是动画模型
    bool isAnimModel() const {
        return model->IsAnimated();
    }
};