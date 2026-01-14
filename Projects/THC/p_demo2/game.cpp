#include "game.h"
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/Projection.h> 
#include <custom/ModelTrans.h> 


#include <custom/shader_m.h>
#include <custom/renderer.h>

#include "custom/model_anim_data.h"
#include "custom/animation.h"
#include "custom/animator.h"
#include <custom/model_base.h>
#include "custom/model_animated.h"
#include <custom/model_static.h>
#include <games/object.h>
#include <games/character.h>
#include <games/enemy.h>
#include <games/CombatSystem.h>
#include <custom/timer.h>
#include <games/PlayController.h>
#include <UI/iui.h>


static void init() {
#pragma region openGL 基础设置
    // 1. 初始化窗口 (利用 custom/window.h)
    // ------------------------------------
    Window window(1920, 1080, "OpenGL Scene - Sky & Ground Test");

    // 2. 设置摄像机和输入控制 (利用 custom/Camera.h 和 InputController.h)
    // ------------------------------------------------------------------
    Camera camera(glm::vec3(0.0f, 0.2f, 3.0f));
    // 速度设为 2.5f, 鼠标灵敏度 0.1f
    InputController controller(camera, 0.5f, 0.1f);

    // 3. 设置投影矩阵 (利用 custom/Projection.h)
    // -----------------------------------------
    Projection projection(45.0f, 0.001f, 100.0f, 1920.0f, 1080.0f);

    // 4. 编译着色器
    // ------------------------------------
    std::string shaderDir = "../../../shaders/";

    // 使用 Blinn-Phong shader 渲染地面和静态模型
    Shader blinnPhongShader((shaderDir + "Blinn_phong.vs").c_str(), (shaderDir + "Blinn_phong.fs").c_str());

    // 5. 设置地面顶点数据 (包含位置、法线、纹理坐标)
    // ------------------------------------------------------------------
    // 地面法线向上 (0, 1, 0)
    float groundVertices[] = {
        // 位置 (x, y, z)    法线 (nx, ny, nz)    纹理坐标 (u, v)
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f
    };

    unsigned int groundVBO, groundVAO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    // 属性 0: 位置 (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 属性 1: 法线 (vec3)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 属性 2: 纹理坐标 (vec2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 创建简单的白色纹理用于地面
    unsigned int groundTexture;
    glGenTextures(1, &groundTexture);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    // 创建一个 1x1 的白色纹理
    unsigned char whiteData[] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 设置点光源（使用Renderer::PointLight结构）
    std::vector<Renderer::PointLight> pointLights;
    pointLights.push_back(Renderer::PointLight(
        glm::vec3(-0.19f, 0.11f, -0.38f), glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, 1.0f, 0.09f, 0.032f));  // 光源1：白色
    pointLights.push_back(Renderer::PointLight(
        glm::vec3(0.19f, 0.11f, -1.50f), glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, 1.0f, 0.09f, 0.032f));  // 光源2：淡红色（关闭）
    pointLights.push_back(Renderer::PointLight(
        glm::vec3(1.60f, 0.07f, 1.41f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f, 1.0f, 0.09f, 0.032f));      // 光源3：淡蓝色（关闭）
    pointLights.push_back(Renderer::PointLight(
        glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.8f), 0.0f, 1.0f, 0.09f, 0.032f));      // 光源4：淡黄色（关闭）

    float globalLightIntensity = 1.0f;  // 全局光强乘数（统一调整所有光源）

    // 材质设置
    Renderer::Material groundMaterial(
        glm::vec3(0.2f, 0.2f, 0.2f),  // ambient
        glm::vec3(0.5f, 0.5f, 0.5f),  // specular
        32.0f,                         // shininess
        false                          // useTextureDiffuse1（地面使用material.texture_diffuse）
    );

    Renderer::Material modelMaterial(
        glm::vec3(0.3f, 0.3f, 0.3f),  // ambient
        glm::vec3(0.8f, 0.8f, 0.8f),  // specular
        64.0f,                         // shininess
        true                           // useTextureDiffuse1（模型使用texture_diffuse1）
    );

#pragma endregion

#pragma region imgui 基础设置

    Iui iui(window.get(), 1920, 1080);

#pragma endregion
}

