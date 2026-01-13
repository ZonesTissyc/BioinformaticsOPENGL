#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/Projection.h> 
#include <custom/ModelTrans.h>

#include <custom/shader.h>
#include <learnopengl/model.h>
#include <UI/iui.h>

// 全局计时变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

#pragma region openGL 基础设置
    // 1. 初始化窗口
    Window window(1280, 720, "OpenGL Scene - Phong Lighting Test");

    // 2. 设置摄像机和输入控制
    Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
    InputController controller(camera, 2.5f, 0.1f);

    // 3. 设置投影矩阵
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);

    // 4. ImGui 设置
    Iui iui(window.get());

    // 5. 编译着色器
    std::string rootURL = R"(../../../)";
    std::string vsURL = rootURL + "shaders/phong.vs";
    std::string fsURL = rootURL + "shaders/phong.fs";
    Shader shaderModel(vsURL.c_str(), fsURL.c_str());

    // 6. 地面和光源立方体顶点数据
    float vertices[] = {
        // --- 地面平面 (6个顶点) ---
        25.0f, -0.5f,  25.0f, -25.0f, -0.5f,  25.0f, -25.0f, -0.5f, -25.0f,
        25.0f, -0.5f,  25.0f, -25.0f, -0.5f, -25.0f,  25.0f, -0.5f, -25.0f,
        // --- 立方体 (36个顶点) ---
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 属性 0: 位置 (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 定义点光源
    glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

#pragma endregion

    // 加载模型
    std::string modelPath = rootURL + "resources/model/swimming_pool/swimming_pool_3d_scene.glb";
    Model model(modelPath.c_str());

    // 设置模型变换
    ModelTrans modelTrans;
    modelTrans.translate(glm::vec3(0.0f, 0.0f, 0.0f));
    modelTrans.scale(glm::vec3(1.0f) * 0.5f);

    // 计时
    float lastFrame = static_cast<float>(glfwGetTime());

    // 开启混合和深度测试
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // 渲染循环
    while (window.noClose()) {
        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入
        controller.processKeyboardInput(window.get(), deltaTime);
        controller.processMouseInput(window.get());

        // 渲染设置
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 获取矩阵
        glm::mat4 projMat = projection.getProjection();
        glm::mat4 viewMat = camera.getView();

        // 绘制模型 - 使用Phong光照明模型
        shaderModel.use();
        
        // 设置矩阵
        shaderModel.setMat4("projection", projMat);
        shaderModel.setMat4("view", viewMat);
        shaderModel.setMat4("model", modelTrans.getModelMatrix());
        
        // 设置光照参数
        shaderModel.setVec3("lightPos", lightPos);
        shaderModel.setVec3("lightColor", lightColor);
        shaderModel.setVec3("viewPos", camera.getPos());
        
        // 设置材质属性
        shaderModel.setVec3("materialSpecular", 0.5f, 0.5f, 0.5f);
        shaderModel.setFloat("materialShininess", 32.0f);
        
        // 设置纹理参数
        shaderModel.setBool("hasTexture", true);
        shaderModel.setVec4("materialColor", 1.0f, 1.0f, 1.0f, 1.0f);
        
        // 设置自发光参数
        shaderModel.setBool("hasEmissiveMap", false);
        shaderModel.setVec3("emissiveColor", 0.0f, 0.0f, 0.0f);
        
        // 绘制模型
        model.Draw(shaderModel, camera.getPos());

        // ImGui显示 - 放在所有渲染之后
        iui.beginFrame();
        iui.showFPS();
        iui.showPos(camera.getPos());
        iui.endFrame();

        // 交换缓冲区
        window.swapBuffers();
    }

    return 0;
}