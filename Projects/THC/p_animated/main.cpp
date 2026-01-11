#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

// 引入您项目中的自定义头文件
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/Projection.h> 
#include <custom/ModelTrans.h> 


#include <custom/shader_m.h>

#include "custom/model_anim_data.h"
#include "custom/animation.h"
#include "custom/animator.h"
#include <custom/model_base.h>
#include "custom/model_animated.h"
#include <games/object.h>
#include <games/character.h>

#include <custom/timer.h>
// 全局计时变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // 1. 初始化窗口 (利用 custom/window.h)
    // ------------------------------------
    Window window(1280, 720, "OpenGL Scene - Sky & Ground Test");

    // 2. 设置摄像机和输入控制 (利用 custom/Camera.h 和 InputController.h)
    // ------------------------------------------------------------------
    Camera camera(glm::vec3(0.0f, 0.2f, 3.0f));
    // 速度设为 2.5f, 鼠标灵敏度 0.1f
    InputController controller(camera, 0.5f, 0.1f);

    // 3. 设置投影矩阵 (利用 custom/Projection.h)
    // -----------------------------------------
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);

    // 4. 编译着色器
    // ------------------------------------
    // 注意：路径取决于您的 exe 运行位置。通常在 IDE 中运行时，相对路径是相对于项目文件或解决方案文件的。
    // 这里假设从 Projects/THC/p_sky1/ 运行，需要回溯到根目录找到 shaders 文件夹。
    std::string shaderDir = "../../../shaders/";

    // 使用现有的 colors shader 渲染地面
    Shader colorShader((shaderDir + "1.colors/1.colors.vs").c_str(), (shaderDir + "1.colors/1.colors.fs").c_str());
    // 使用现有的 light_cube shader 渲染光源立方体
    Shader lightShader((shaderDir + "1.colors/1.light_cube.vs").c_str(), (shaderDir + "1.colors/1.light_cube.fs").c_str());

    // 5. 设置顶点数据 (地面平面 + 光源立方体)
    // ------------------------------------------------------------------
    float vertices[] = {
        // --- 地面平面 (6个顶点) ---
        // 位置 (x, y, z)            
         25.0f, -0.5f,  25.0f,
        -25.0f, -0.5f,  25.0f,
        -25.0f, -0.5f, -25.0f,

         25.0f, -0.5f,  25.0f,
        -25.0f, -0.5f, -25.0f,
         25.0f, -0.5f, -25.0f,

         // --- 立方体 (用于显示光源位置, 36个顶点) ---
         -0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,
         -0.5f,  0.5f, -0.5f,
         -0.5f, -0.5f, -0.5f,

         -0.5f, -0.5f,  0.5f,
          0.5f, -0.5f,  0.5f,
          0.5f,  0.5f,  0.5f,
          0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f,  0.5f,
         -0.5f, -0.5f,  0.5f,

         -0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f, -0.5f,
         -0.5f, -0.5f, -0.5f,
         -0.5f, -0.5f, -0.5f,
         -0.5f, -0.5f,  0.5f,
         -0.5f,  0.5f,  0.5f,

          0.5f,  0.5f,  0.5f,
          0.5f,  0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f, -0.5f,  0.5f,
          0.5f,  0.5f,  0.5f,

         -0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f, -0.5f,  0.5f,
          0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f, -0.5f,

         -0.5f,  0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,
          0.5f,  0.5f,  0.5f,
          0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f, -0.5f
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

    // 光源位置
    glm::vec3 lightPos(44.2f, 1.0f, 2.0f);


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
    std::string glb8 = "swimming_pool_3d_scene.glb";
    // 加载模型（骨骼）与动画（从同一 glb 文件读取）
    std::string glbPath = glb2;

    auto modelData = std::make_shared<ModelAnimData>(glbPath);
    Animation* animation = new Animation(glbPath, modelData.get(),10);
    auto modelAnimated = std::make_shared<ModelAnimated>(modelData, std::shared_ptr<Animation>(animation));

    auto playerModel = ModelAnimated::LoadModelWithAllAnimations(glbPath);
    Character player2(playerModel.get(), &shader1, glm::vec3(0.0f, 0.2f, 3.0f));
    player2.SetAction(Character::Action::Run, false);
    controller.setCharacter(&player2);


    ModelTrans transmat;
    transmat.scale(glm::vec3(1.0f, 1.0f, 1.0f)*6.0F);
    // transmat.rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    // 着色器预设（投影可在窗口大小变化时更新）
    ModelTrans transmat2;
    transmat2.translate(player2.position);

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
        // 处理输入
        controller.processKeyboardInput(window.get(), dt);
        controller.processMouseInput(window.get());

        // 渲染设置
        // 设置清屏颜色 (天空颜色: 天蓝色)
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 获取矩阵
        glm::mat4 projMat = projection.getProjection();
        glm::mat4 viewMat = camera.getView();
        glm::mat4 modelmat = glm::mat4(1.0f);

        // --- 1. 绘制地面 ---
        colorShader.use(); 
        colorShader.setMat4("projection", projMat);
        colorShader.setMat4("view", viewMat);

        modelmat = glm::mat4(1.0f);
        colorShader.setMat4("model", modelmat);

        // 设置颜色：地面为深灰色，光照为白色
        colorShader.setVec3("objectColor", 0.3f, 0.3f, 0.3f);
        colorShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

        glBindVertexArray(VAO);
        // 绘制前6个顶点 (地面)
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 2. 绘制光源 (小立方体) ---
        lightShader.use();
        lightShader.setMat4("projection", projMat);
        lightShader.setMat4("view", viewMat);

        modelmat = glm::mat4(1.0f);
        modelmat = glm::translate(modelmat, lightPos);
        modelmat = glm::scale(modelmat, glm::vec3(0.2f)); // 缩小立方体
        lightShader.setMat4("model", modelmat);

        
        // 绘制模型
		Renderer::BeginScene(camera, projMat, shader1);
        player2.Update(deltaTime);
		player2.Draw(shader1);
		
        Renderer::EndScene();

        glDrawArrays(GL_TRIANGLES, 6, 36);

        // 交换缓冲区
        window.swapBuffers();
    }

    return 0;
}