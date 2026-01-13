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

// ȫ�ּ�ʱ����
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

#pragma region openGL ��������
    // 1. ��ʼ������
    Window window(1280, 720, "OpenGL Scene - Phong Lighting Test");

    // 2. ������������������
    Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
    InputController controller(camera, 2.5f, 0.1f);

    // 3. ����ͶӰ����
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);

    // 4. ImGui ����
    Iui iui(window.get());

    // 5. ������ɫ��
    std::string rootURL = R"(../../../)";
    std::string vsURL = rootURL + "shaders/phong.vs";
    std::string fsURL = rootURL + "shaders/phong.fs";
    Shader shaderModel(vsURL.c_str(), fsURL.c_str());

    // 6. ����͹�Դ�����嶥������
    float vertices[] = {
        // --- ����ƽ�� (6������) ---
        25.0f, -0.5f,  25.0f, -25.0f, -0.5f,  25.0f, -25.0f, -0.5f, -25.0f,
        25.0f, -0.5f,  25.0f, -25.0f, -0.5f, -25.0f,  25.0f, -0.5f, -25.0f,
        // --- ������ (36������) ---
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

    // ���� 0: λ�� (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ������Դ����
    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        float radius;
    };
    
    // �������Դ
    std::vector<PointLight> pointLights = {
        {{2.0f, 5.0f, 2.0f}, {1.0f, 0.0f, 0.0f}, 1.0f, 0.5f},    // ��ɫԴ
        {{-3.0f, 4.0f, -3.0f}, {0.0f, 1.0f, 0.0f}, 1.0f, 0.5f},  // ��ɫԴ
        {{3.0f, 6.0f, -4.0f}, {0.0f, 0.0f, 1.0f}, 1.0f, 0.5f},   // ��ɫԴ
        {{-2.0f, 7.0f, 3.0f}, {1.0f, 1.0f, 0.0f}, 1.0f, 0.5f}    // �������
    };
    
    // ��ɫԴͶӰ��ɫ��
    Shader lightCubeShader("../../../shaders/1.colors/1.light_cube.vs", "../../../shaders/1.colors/1.light_cube.fs");

#pragma endregion

    // ����ģ��
    std::string modelPath = rootURL + "resources/model/swimming_pool/swimming_pool_3d_scene.glb";
    Model model(modelPath.c_str());

    // ����ģ�ͱ任
    ModelTrans modelTrans;
    modelTrans.translate(glm::vec3(0.0f, 0.0f, 0.0f));
    modelTrans.scale(glm::vec3(1.0f) * 0.5f);

    // ��ʱ
    float lastFrame = static_cast<float>(glfwGetTime());

    // ������Ϻ���Ȳ���
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // ��Ⱦѭ��
    while (window.noClose()) {
        // ����֡ʱ��
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ��������
        controller.processKeyboardInput(window.get(), deltaTime);
        controller.processMouseInput(window.get());

        // ��Ⱦ����
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ��ȡ����
        glm::mat4 projMat = projection.getProjection();
        glm::mat4 viewMat = camera.getView();

        // ����ģ�� - ʹ��Phong������ģ��
        shaderModel.use();

        // ���þ���
        shaderModel.setMat4("projection", projMat);
        shaderModel.setMat4("view", viewMat);
        shaderModel.setMat4("model", modelTrans.getModelMatrix());

        // ���ù��ղ���
        shaderModel.setVec3("lightPos", lightPos);
        shaderModel.setVec3("lightColor", lightColor);
        shaderModel.setVec3("viewPos", camera.getPos());

        // ���ò�������
        shaderModel.setVec3("materialSpecular", 0.5f, 0.5f, 0.5f);
        shaderModel.setFloat("materialShininess", 32.0f);

        // ������������
        shaderModel.setBool("hasTexture", true);
        shaderModel.setVec4("materialColor", 1.0f, 1.0f, 1.0f, 1.0f);

        // �����Է������
        shaderModel.setBool("hasEmissiveMap", false);
        shaderModel.setVec3("emissiveColor", 0.0f, 0.0f, 0.0f);

        // ����ģ��
        model.Draw(shaderModel, camera.getPos());

        // ImGui��ʾ - ����������Ⱦ֮��
        iui.beginFrame();
        iui.showFPS();
        iui.showPos(camera.getPos());
        iui.endFrame();

        // ����������
        window.swapBuffers();
    }

    return 0;
}