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

#include <learnopengl/shader.h>
// #include <learnopengl/model.h>
#include "model_PBR.h"

// 全局计时变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

#pragma region 初始化
    Window window(1280, 720, "OpenGL Scene - Sky & Ground Test");
    Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
    InputController controller(camera, 2.5f, 0.1f);
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);

    std::string shaderDir = "../../../shaders/";
    Shader colorShader((shaderDir + "1.colors/1.colors.vs").c_str(), (shaderDir + "1.colors/1.colors.fs").c_str());
    Shader lightShader((shaderDir + "1.colors/1.light_cube.vs").c_str(), (shaderDir + "1.colors/1.light_cube.fs").c_str());

    float vertices[] = {
        // ... (保持你原来的顶点数据不变，这里为了省略篇幅略去，代码中请保留) ...
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 光源位置
    glm::vec3 lightPos(4.0f, 4.0f, 4.0f); // [修改] 稍微调整光的位置让它离模型近一点，方便观察

#pragma endregion

    std::string rootURL = R"(../../../)";
    std::string vsURL = rootURL + "shaders/model_light.vs";
    std::string fsURL = rootURL + "shaders/model_PBR.fs";
    Shader shaderModel(vsURL.c_str(), fsURL.c_str());

    std::string glb1 = "swimming_pool_3d_scene.glb";
    std::string glb2 = "sci-fi_lab.glb";
    std::string glb3 = "abandoned_warehouse_-_interior_scene.glb";
    std::string glb4 = "minecraft_steave.glb";
    std::string glb5 = "mambo.glb";
    // stbi_set_flip_vertically_on_load(true);
    Model model_1(glb1.c_str());

    ModelTrans modelTrans;
    modelTrans.translate(glm::vec3(0.0f,4.0f, -1.0f)); // 稍微调整位置
    modelTrans.scale(glm::vec3(1.0f) * 1.0f);           // 调整缩放
    // modelTrans.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    float lastFrame = static_cast<float>(glfwGetTime());

    // ------------------------------------------------------------------
    // [修改] 1. 开启混合 (水面透明) 和 深度测试 (解决模型错位)
    // ------------------------------------------------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); // <--- [关键] 必须开启，否则后面的墙会挡住前面的扶手

    // 6. 渲染循环
    while (window.noClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        controller.processKeyboardInput(window.get(), deltaTime);
        controller.processMouseInput(window.get());

        // [修改] 2. 清屏时清除 颜色缓冲 和 深度缓冲
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projMat = projection.getProjection();
        glm::mat4 viewMat = camera.getView();
        glm::mat4 modelmat1 = glm::mat4(1.0f);

        // --- 绘制地面 ---
        colorShader.use();
        colorShader.setMat4("projection", projMat);
        colorShader.setMat4("view", viewMat);
        colorShader.setMat4("model", glm::mat4(1.0f));
        colorShader.setVec3("objectColor", 0.3f, 0.3f, 0.3f);
        colorShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 绘制光源 (小立方体) ---
        // [修改] 3. 修正光源绘制逻辑，确保在 lightShader 激活时绘制
        lightShader.use();
        lightShader.setMat4("projection", projMat);
        lightShader.setMat4("view", viewMat);
        modelmat1 = glm::mat4(1.0f);
        modelmat1 = glm::translate(modelmat1, lightPos);
        modelmat1 = glm::scale(modelmat1, glm::vec3(0.2f));
        lightShader.setMat4("model", modelmat1);

        // [关键] 绘制光源立方体必须在这里调用，用 lightShader 绘制
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 6, 36);

        // --- 绘制模型 ---
        shaderModel.use();
        shaderModel.setMat4("projection", projMat);
        shaderModel.setMat4("view", viewMat);
        shaderModel.setMat4("model", modelTrans.getModelMatrix());

        // [修改] 4. 传递光照所需的 Uniforms (对应新的 fs 代码)
        shaderModel.setVec3("lightPos", lightPos);
        shaderModel.setVec3("viewPos", camera.getPos()); // 假设 Camera 类有 Position 成员变量

        model_1.Draw(shaderModel, camera.getPos());

        window.swapBuffers();
    }

    return 0;
}