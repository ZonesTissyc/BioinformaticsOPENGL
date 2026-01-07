#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>

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

#pragma region 
    Window window(1280, 720, "Blinn-phong");
    Camera camera(glm::vec3(-1.0f, 3.65f, -4.0f));
    InputController controller(camera, 0.5f, 0.1f);
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);
    Iui iui(window.get());
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

    // 点光源
    glm::vec3 lightPos(-2.73f, 5.49f, -6.49f); 

#pragma endregion

    std::string rootURL = R"(../../../)";
    std::string vsURL = rootURL + "shaders/Blinn_phong.vs"; 
    std::string fsURL = rootURL + "shaders/Blinn_phong.fs"; 
    Shader shaderModel(vsURL.c_str(), fsURL.c_str());

    std::string resourcesURL = rootURL + "resources/model/";
    std::string glb1 =resourcesURL + "swimming_pool/swimming_pool_3d_scene.glb";
    
    // stbi_set_flip_vertically_on_load(true);
    Model model_1(glb1.c_str());

    ModelTrans modelTrans;
    modelTrans.translate(glm::vec3(0.0f,4.0f, -1.0f)); // 稍微调整位置
    modelTrans.scale(glm::vec3(1.0f) * 1.0f);           // 调整缩放
    // modelTrans.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    float lastFrame = static_cast<float>(glfwGetTime());

    // [修改] 1. 定义多点光源
    // ------------------------------------------------------------------
    // 定义4个点光源的位置（放置在场景的四个角落）
    std::vector<glm::vec3> lightPositions = {
        glm::vec3(-2.73f, 5.49f, -6.49f),  // 左上角
        glm::vec3(2.73f, 5.49f, -6.49f),   // 右上角
        glm::vec3(-2.73f, 5.49f, 4.51f),   // 左下角
        glm::vec3(2.73f, 5.49f, 4.51f)     // 右下角
    };
    
    // 定义4个点光源的颜色（不同颜色以区分光源）
    std::vector<glm::vec3> lightColors = {
        glm::vec3(1.0f, 1.0f, 1.0f),  
        glm::vec3(1.0f, 1.0f, 1.0f),   
        glm::vec3(1.0f, 1.0f, 1.0f),   
        glm::vec3(1.0f, 1.0f, 1.0f)    
    };
    
    // 设置光源数量
    int numLights = lightPositions.size();
    // ------------------------------------------------------------------
    // [修改] 2. 开启混合 (水面透明) 和 深度测试 (解决模型错位)
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
        // [修改] 3. 修正光源绘制逻辑，为每个点光源绘制一个小立方体
        lightShader.use();
        lightShader.setMat4("projection", projMat);
        lightShader.setMat4("view", viewMat);
        
        // 为每个点光源绘制一个小立方体
        for (const auto& lightPos : lightPositions) {
            modelmat1 = glm::mat4(1.0f);
            modelmat1 = glm::translate(modelmat1, lightPos);
            modelmat1 = glm::scale(modelmat1, glm::vec3(0.005f)); // 进一步缩小光源立方体
            lightShader.setMat4("model", modelmat1);

            // [关键] 绘制光源立方体必须在这里调用，用 lightShader 绘制
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 6, 36);
        }

        // --- 绘制模型 ---
        shaderModel.use();
        shaderModel.setMat4("projection", projMat);
        shaderModel.setMat4("view", viewMat);
        shaderModel.setMat4("model", modelTrans.getModelMatrix());

        // 设置材质属性
        shaderModel.setVec3("material.ambient", 0.1f, 0.1f, 0.1f);
        shaderModel.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        shaderModel.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        shaderModel.setFloat("material.shininess", 32.0f);
        
        // 设置光源数量
        shaderModel.setInt("numLights", numLights);
        
        // 设置每个点光源的属性
        for (int i = 0; i < numLights; i++) {
            // 构建光源属性名称
            std::string lightPrefix = "pointLights[" + std::to_string(i) + "]";
            
            // 位置
            shaderModel.setVec3(lightPrefix + ".position", lightPositions[i]);
            
            // 颜色和强度（不同颜色以区分光源）
            shaderModel.setVec3(lightPrefix + ".color", lightColors[i]);
            shaderModel.setFloat(lightPrefix + ".intensity", 1.0f);
            
            // 衰减参数（统一设置，可根据需要调整）
            shaderModel.setFloat(lightPrefix + ".constant", 1.0f);
            shaderModel.setFloat(lightPrefix + ".linear", 0.09f);
            shaderModel.setFloat(lightPrefix + ".quadratic", 0.032f);
        }
        
        // 设置视图位置
        shaderModel.setVec3("viewPos", camera.getPos());
        
        model_1.Draw(shaderModel, camera.getPos());
        
        // imgui开始
        iui.beginFrame();
        iui.showFPS();
        iui.showPos(camera.getPos());
        //iui.drawCrosshair();
        iui.endFrame();

        window.swapBuffers();
    }

    return 0;
}