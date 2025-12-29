#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept> //仅抛出异常

class Window {
public:
    Window(int width, int height, const char* title) {
        initGLFW();
        createWindow(width, height, title);
        initGLAD();

        initCallbacks();
    }

    bool noClose() const {
        return !glfwWindowShouldClose(window);
	}

    void swapBuffers(bool isPoll = true)
    {
        glfwSwapBuffers(window);
        if (isPoll)
        {
            glfwPollEvents();
        }
    }

    ~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow* get() const { return window; }
    void setSize(int width, int height) {
        glfwSetWindowSize(window, width, height);
    }

private:
    GLFWwindow* window = nullptr;

    void initGLFW() {
        if (!glfwInit())
            throw std::runtime_error("GLFW init failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x超采样
    }

    void createWindow(int w, int h, const char* title, bool getMouse = true) {
        window = glfwCreateWindow(w, h, title, nullptr, nullptr);
        if (!window)
            throw std::runtime_error("Window creation failed");

        glfwMakeContextCurrent(window);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // 捕获鼠标输入

    }

    void initGLAD() {
        static bool gladInitialized = false;
        if (!gladInitialized) {
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
                throw std::runtime_error("GLAD init failed");
            gladInitialized = true;
        }
		glEnable(GL_DEPTH_TEST);    // 默认启用深度测试
    }

    

    #pragma region 窗口可调节回调函数
    void initCallbacks() {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    }

    static void framebuffer_size_callback(GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    }
    #pragma endregion
};
