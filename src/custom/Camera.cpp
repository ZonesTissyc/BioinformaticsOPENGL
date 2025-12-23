// Camera.cpp v 1.2
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 Camera::getRight() const {
    // Right = Cross(Front, Up)
    // 注意：我们假设 up_ 始终是世界 up (0, 1, 0)
    // 这里需要将 up_ 视为世界 up，以避免 roll 旋转。
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // 叉乘结果必须归一化
    return glm::normalize(glm::cross(front_, worldUp));
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(pos_, pos_ + front_, up_);
}

Camera::~Camera()
{

}
