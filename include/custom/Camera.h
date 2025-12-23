// Camera v 1.2
#pragma once 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Camera {
private:
    glm::vec3 pos_ = glm::vec3(0.0f, 0.0f, 1.5f);
    glm::vec3 front_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float sensitivity_ = 0.1f;

    void updateCameraVectors() {
        // 计算新的方向向量
        glm::vec3 front;
        front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));

        front_ = glm::normalize(front);
        // 重新计算 Right 和 Up，确保正交
        // 假设世界向上向量为 (0,1,0)
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(front_, worldUp));
        up_ = glm::normalize(glm::cross(right, front_));
    }
public:
    Camera() {

    }
    Camera(glm::vec3 pos_origin) {
        pos_ = pos_origin;
    }
    void setPos(glm::vec3 pos_tochange) {
        pos_ = pos_tochange;
    }
    void setFront(glm::vec3 front_tochange) {
        front_ = front_tochange;
    }
    void setUp(glm::vec3 up_tochange) {
        up_ = up_tochange;
    }

    glm::vec3 getPos() const {
        return pos_;
    }
    glm::vec3 getFront() const {
        return front_;
    }
    glm::vec3 getUp() const {
        return up_;
    }
    glm::vec3 getRight() const;
    glm::mat4 getView() const;

    void processMouseMovement(float xoffset, float yoffset) {
        xoffset *= sensitivity_;
        yoffset *= sensitivity_;

        yaw_ += xoffset;
        pitch_ += yoffset;

        // 约束俯仰角，防止“翻跟头”导致画面颠倒
        if (pitch_ > 89.0f)  pitch_ = 89.0f;
        if (pitch_ < -89.0f) pitch_ = -89.0f;

        updateCameraVectors();
    }
    ~Camera();
};

