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

// 引入 LearnOpenGL 的 Shader 类
#include <custom/shader_m.h>

#include "games/model_anim_data.h"
#include "games/animation.h"
#include "games/animator.h"
#include "games/model_animated.h"

#include <custom/timer.h>

int main() {
    Window window(1280, 720, "OpenGL Scene - Sky & Ground Test");
    Camera camera(glm::vec3(0.0f, 0.2f, 3.0f));
    InputController controller(camera, 0.5f, 0.1f);
    Projection projection(45.0f, 0.1f, 100.0f, 1280.0f, 720.0f);

    std::string rootURL = "../../../";
    std::string shaderURL = rootURL + "shaders/";
    std::string shaderM_vs = shaderURL + "anim_model.vs";
    std::string shaderM_fs = shaderURL +"anim_model.fs";

    Shader shaderModel(shaderM_vs.c_str(), shaderM_fs.c_str());

    std::string glb1URL = rootURL + "resources/";
    std::string glb_majar = glb1URL + "model/major-solder/major-solder.glb";
    
    auto modelData = std::make_shared<ModelAnimData>(glb_majar);
    Animation* animation = new Animation(glb_majar, modelData.get());
    auto modelAnimated = std::make_shared<ModelAnimated>(modelData, std::shared_ptr<Animation>(animation));

    Timer timer;

    while (window.noClose()){
        timer.tick();
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        controller.processMouseInput(window.get());
        controller.processMouseInput(window.get());
        shaderModel.use();

        // 上传相机矩阵（custom::Camera 提供 getView()）
        shaderModel.setMat4("view", camera.getView());
        shaderModel.setMat4("projection", glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f));

        // 模型变换
        glm::mat4 modelMat = glm::mat4(1.0f);
        shaderModel.setMat4("model", modelMat);

        modelAnimated->Update(timer.deltaTime());

        // 绘制模型
        modelAnimated->Update(timer.deltaTime());
        modelAnimated->Draw(shaderModel, camera.getPos());
        window.swapBuffers();
    }
}