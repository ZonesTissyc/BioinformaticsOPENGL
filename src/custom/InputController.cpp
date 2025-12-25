// InputController v 1.2
#include <custom/InputController.h>
#include <glm/glm.hpp>

// 核心方法实现
void InputController::processKeyboardInput(GLFWwindow* window, float deltaTime) {
    // 退出判断
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 计算当前帧的移动速度
    float velocity = cameraSpeed_ * deltaTime;

    // 获取当前摄像机状态
    glm::vec3 pos = controlledCamera_.getPos();
    glm::vec3 front = controlledCamera_.getFront();
    glm::vec3 right = controlledCamera_.getRight();
    glm::vec3 up = controlledCamera_.getUp();

    // W 键: 前进
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        pos += front * velocity;
    }
    // S 键: 后退
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        pos -= front * velocity;
    }
    // D 键: 右平移
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        pos += right * velocity;
    }
    // A 键: 左平移
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        pos -= right * velocity;
    }

    // Z 键: 向上
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        pos += up * velocity;
    }

    // 返回实例
    controlledCamera_.setPos(pos);
}

void InputController::processMouseInput(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos); // 获取当前鼠标在窗口的坐标

    // 第一帧处理：防止视角猛然跳变
    if (firstMouse_) {
        lastX_ = static_cast<float>(xpos);
        lastY_ = static_cast<float>(ypos);
        firstMouse_ = false;
    }

    // 1. 计算偏移量
    float xoffset = static_cast<float>(xpos) - lastX_;
    float yoffset = lastY_ - static_cast<float>(ypos); // 注意：y坐标是从下往上算的，所以要反过来

    lastX_ = static_cast<float>(xpos);
    lastY_ = static_cast<float>(ypos);

    // 2. 应用灵敏度
    xoffset *= sensitivity_;
    yoffset *= sensitivity_;

    // 3. 调用 Camera 类的旋转方法（假设你在 Camera 类中实现了 processMouseMovement）
    controlledCamera_.processMouseMovement(xoffset, yoffset);
}
