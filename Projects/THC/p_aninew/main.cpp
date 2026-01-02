#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/Projection.h>
#include <learnopengl/shader.h>
#include "model_new.h"
#include "animation.h"
#include "animator.h"

int main() {
    #pragma region "初始化"
    // 1. 初始化窗口 (利用 custom/window.h)
    // ------------------------------------
    Window window(1280, 720, "OpenGL Scene - Sky & Ground Test");

    // 2. 设置摄像机和输入控制 (利用 custom/Camera.h 和 InputController.h)
    // ------------------------------------------------------------------
    Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
    // 速度设为 2.5f, 鼠标灵敏度 0.1f
    InputController controller(camera, 2.5f, 0.1f);

    // 3. 设置投影矩阵 (利用 custom/Projection.h)
    // -----------------------------------------
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);
    #pragma endregion
    std::string rootPath = "../../../";
    std::string model_vs = rootPath + "shaders/new_model.vs";
    std::string model_fs =rootPath + "shaders/new_model.fs";
    Shader shader_model(model_vs.c_str(), model_fs.c_str());
    std::string glb1 = "gltf_to_glb1.glb";
    std::string glb2 = rootPath + "resources/model/npc-solder1/npc-solder1.glb";
    std::string glbPath = glb2;
    Model model(glbPath.c_str());
    Animation animation(glbPath, &model);
    Animator animator(&animation);

    // 时间变量
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    glEnable(GL_DEPTH_TEST);
    while (window.noClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        controller.processKeyboardInput(window.get(), deltaTime);
        animator.UpdateAnimation(deltaTime);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 深灰色背景
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_model.use();

        // 1. 传递 View 和 Projection 矩阵
        shader_model.setMat4("projection", projection.getProjection());
        shader_model.setMat4("view", camera.getView());

        // 2. 传递 Model 矩阵 (调整模型位置)
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // 放在原点
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));     // 根据需要缩放，如果模型太小改为 10.0f，太大改为 0.1f
        shader_model.setMat4("model", modelMatrix);

        // 3. 【最重要】传递骨骼变换矩阵数组
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
        {
            // Uniform 名字必须与 Shader 中一致： finalBonesMatrices[i]
            shader_model.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        // 4. 绘制
        model.Draw(shader_model);
    }
}