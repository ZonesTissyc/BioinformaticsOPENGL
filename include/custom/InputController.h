// InputController v 1.2
#pragma once

#include <GLFW/glfw3.h>
#include <custom/Camera.h> // 需要包含 Camera 类的头文件

class InputController {
private:
    // 存储 Camera 对象的引用
    Camera& controlledCamera_;

    float cameraSpeed_; // 移动速率

    // --- 新增鼠标控制参数 ---
    float lastX_ = 400.0f;       // 上一帧鼠标 X 坐标（初始值设为窗口中心）
    float lastY_ = 300.0f;       // 上一帧鼠标 Y 坐标
    bool firstMouse_ = true;     // 是否是第一次接收鼠标输入
    float sensitivity_ = 0.1f;   // 鼠标灵敏度
public:
   
    InputController(Camera& camera, float speed)
        : controlledCamera_(camera), cameraSpeed_(speed) {
    }

    // 处理键盘输入
    void processKeyboardInput(GLFWwindow* window, float deltaTime);
    // 处理鼠标（旋转）
    void processMouseInput(GLFWwindow* window);
};