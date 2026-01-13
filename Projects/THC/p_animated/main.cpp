#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// 引入您项目中的自定义头文件
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

// 全局计时变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

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


    std::string rootURL = R"(../../../)";
    std::string vsURL = rootURL + "shaders/anim_model.vs";
    std::string fsURL = rootURL + "shaders/anim_model.fs";
    Shader shader1(vsURL.c_str(), fsURL.c_str());
    stbi_set_flip_vertically_on_load(true);
    // 使用 custom::Camera（不包含 learnopengl 的 camera.h，避免重定义）
   
    std::string glb1 = rootURL + "resources/model/npc-solder1/npc-solder1.glb";
    std::string glb2 = rootURL + "resources/model/major-solder/major-solder.glb";
    std::string glb3 = "futuristic_soldier_free _12mb.glb";
    std::string glb4 = "export_solder2.glb";
    std::string glb5 = "fbx_toglb.glb";
    std::string glb6 = "fbx_toglb3.glb";
    std::string glb7 = "gltf_to_glb1.glb";
    std::string glb8 = rootURL + "resources/model/swimming_pool/swimming_pool_3d_scene.glb";
    std::string glb_house = rootURL + "resources/model/warehouse/warehouse.glb";
    // 加载模型（骨骼）与动画（从同一 glb 文件读取）
    std::string glbPath = glb2;

    auto modelData = std::make_shared<ModelAnimData>(glbPath);
    Animation* animation = new Animation(glbPath, modelData.get(),10);
    auto modelAnimated = std::make_shared<ModelAnimated>(modelData, std::shared_ptr<Animation>(animation));

    auto playerModel = ModelAnimated::LoadModelWithAllAnimations(glbPath);
    Character player2(playerModel.get(), &shader1, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // 正确设置 player2 的缩放大小（Character::Draw() 会使用这个 scale）
    player2.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 1.0f);
    
    player2.SetAction(Character::Action::Stay, false);
    controller.setCharacter(&player2);
	// 创建 PlayController，第三个参数是移动速度（单位：单位/秒）
	// 默认值是 2.5f，可以根据需要调整（例如：1.0f 更慢，5.0f 更快）
	PlayController playController(&player2, camera, 0.55f);

	controller.setPlayController(&playController);
	controller.setCharacter(&player2);

    // ============================
    // 创建敌人对象（用于测试）
    // ============================
    auto enemyModel = ModelAnimated::LoadModelWithAllAnimations(glbPath);
    // 敌人位置：在玩家旁边，更容易看到
    // 玩家位置是 (0.82f, 6.25f, -0.92f)，敌人放在玩家右侧前方
    glm::vec3 enemyStartPos = glm::vec3(-0.31f, 0.00f, -0.39f);  // 起始位置
    glm::vec3 patrolPointA = glm::vec3(-0.31f, 0.00f, -0.39f);   // 巡逻点 A（左侧）
    glm::vec3 patrolPointB = glm::vec3(-0.28f, 0.00f, -1.14f);   // 巡逻点 B（右侧）
    
    Enemy enemy(playerModel.get(), &shader1, 
                enemyStartPos,                    // 位置：玩家右侧前方（更容易看到）
                glm::vec3(0.0f, 0.01f, 0.0f),      // 命中中心：在角色上方0.5单位（胸部/头部位置）
                0.01f,                             // 命中半径：0.5单位
                patrolPointA,                     // 巡逻点 A
                patrolPointB,                      // 巡逻点 B
                true);                            // 启用巡逻
    enemy.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 1.0f);
    enemy.yaw = 90.0f;
    enemy.SetAction(Character::Action::Walk, false);  // 初始状态设为行走
    
    // ============================
    // 创建战斗系统
    // ============================
    CombatSystem combatSystem(camera, 100.0f, true);  // 最大射击距离100，启用调试
    combatSystem.AddEnemy(&enemy);  // 将敌人添加到战斗系统

    // 加载静态模型（使用 Blinn-Phong shader）
    stbi_set_flip_vertically_on_load(false);
    std::string glbStatic = glb8;
	auto modelStatic = std::make_shared<ModelStatic>(glbStatic);
	ModelTrans transmatStatic;
    transmatStatic.scale(glm::vec3(1.0f) * 0.07f);
	transmatStatic.translate(glm::vec3(0.0f, 0.0f, 0.0f));

    auto modelHouse = std::make_shared <ModelStatic>(glb_house);
    ModelTrans transnatHouse;
    transnatHouse.scale(glm::vec3(1.0f) * 0.08f);
    transnatHouse.translate(glm::vec3(20.0f, 0.0f,20.0f));
    // 计时
    float lastFrame = static_cast<float>(glfwGetTime());

    // timer类
    Timer timer;
    float dt = 0.0f;
    int timeforani = 0;
    // 6. 渲染循环
    // ------------------------------------------------------------------
    while (window.noClose()) {
        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        dt = timer.tick();
        
        // 先更新ImGui（这样ImGui的IO状态会在处理鼠标输入时可用）
        iui.beginFrame();
        
        // 处理输入（鼠标输入会检查ImGui是否想要捕获鼠标）
        controller.processKeyboardInput(window.get(), dt);
        controller.processMouseInput(window.get());

        // 渲染设置
        // 设置清屏颜色 (深色背景，更好地展示光照效果)
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 获取矩阵
        glm::mat4 projMat = projection.getProjection();
        glm::mat4 viewMat = camera.getView();
        glm::mat4 modelmat = glm::mat4(1.0f);

        // --- 1. 绘制地面 (使用 Blinn-Phong 光照) ---
        blinnPhongShader.use();
        blinnPhongShader.setMat4("projection", projMat);
        blinnPhongShader.setMat4("view", viewMat);
        blinnPhongShader.setMat4("model", modelmat);
        blinnPhongShader.setVec3("viewPos", camera.getPos());

        // 使用Renderer设置材质和光源
        Renderer::SetBlinnPhongMaterial(blinnPhongShader, groundMaterial);
        Renderer::SetBlinnPhongLights(blinnPhongShader, pointLights, globalLightIntensity);

        // 绑定地面纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexture);

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        
        // 绘制模型
		Renderer::BeginScene(camera, projMat, shader1);
        player2.Update(deltaTime);
		player2.Draw(shader1);  // Character::Draw() 内部会设置正确的 model 矩阵（包含 scale）
		
		// 更新并绘制敌人
		enemy.Update(deltaTime);
		enemy.Draw(shader1);
		
		// ============================
		// 处理射击输入（战斗系统）
		// ============================
		combatSystem.ProcessShootInput(window.get());
		

		// 绘制静态模型 (使用 Blinn-Phong 光照)
		blinnPhongShader.use();
		blinnPhongShader.setMat4("projection", projMat);
		blinnPhongShader.setMat4("view", viewMat);
		blinnPhongShader.setVec3("viewPos", camera.getPos());

		// 使用Renderer设置材质和光源
		Renderer::SetBlinnPhongMaterialForModel(blinnPhongShader, modelMaterial);
		Renderer::SetBlinnPhongLights(blinnPhongShader, pointLights, globalLightIntensity);

		Renderer::Submit(blinnPhongShader, modelStatic.get(), transmatStatic.getModelMatrix());
        Renderer::Submit(blinnPhongShader, modelHouse.get(), transnatHouse.getModelMatrix());
        Renderer::EndScene();

        // 显示UI（beginFrame已经在循环开始时调用了）
        iui.showFPS(1.4f);
        iui.showPos(camera.getPos(), 1.2f);
        
        
        // iui.drawCrosshair();
        iui.endFrame();

        // 交换缓冲区
        window.swapBuffers();
    }

    return 0;
}