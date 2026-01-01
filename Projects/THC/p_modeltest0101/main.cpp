#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "model_ani.h"
#include <learnopengl/shader.h>
#include <learnopengl/mesh.h>
// #include <learnopengl/model.h>
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/ModelTrans.h>
#include <custom/Projection.h>

#include <learnopengl/shader.h>
#include <learnopengl/animator.h>


int main() {
	std::string rootURL = R"(../../../shaders/)";
	Window window(1280, 720, "glb_test");
	std::string vsURL = rootURL + "1.model_loading.vs";
	std::string fsURL = rootURL + "1.model_loading.fs";
	Shader shader1(vsURL.c_str(), fsURL.c_str());
	Camera camera(glm::vec3(1.0f, 0.05f, 2.0f));
	InputController controller(camera, 1.0f, 0.2);
	std::string glb1 = "gltf_to_glb1.glb";
	std::string glbURL = glb1;
	Model model(glbURL.c_str(), true);
	ModelTrans matrotate;
    matrotate.scale(glm::vec3(2.0f, 2.0f, 2.0f)* 4.0f);
	// matrotate.rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);
    // std::string rootURL = R"(../../../)";
    std::string vsURL2 = rootURL + "shaders/anim_model.vs";
    std::string fsUR2 = rootURL + "shaders/anim_model.fs";
    Shader shader2(vsURL.c_str(), fsURL.c_str());
    stbi_set_flip_vertically_on_load(true);
    #pragma region 天空
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
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    #pragma endregion


	shader1.use();
	shader1.setMat4("view", camera.getView());
	shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));
	shader1.setMat4("model", matrotate.getModelMatrix());
	while (window.noClose()) {
		controller.processKeyboardInput(window.get(), 0.01); // 更新相机
		controller.processMouseInput(window.get());
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    #pragma region "天空、地面"
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
    #pragma endregion

		shader1.use();
		shader1.setMat4("view", camera.getView());
		shader1.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));
		shader1.setMat4("model", matrotate.getModelMatrix());

		model.Draw(shader1);

		window.swapBuffers();

	}
}