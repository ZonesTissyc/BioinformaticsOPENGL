#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <glm/glm.hpp>

#include <custom/shader.h>
#include <custom/Camera.h>
#include <custom/model_base.h>
#include <custom/model_animated.h> 

class Renderer {
public:
    // 点光源结构体
    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        float constant;
        float linear;
        float quadratic;
        
        // 默认构造函数
        PointLight() : position(0.0f), color(1.0f), intensity(1.0f), constant(1.0f), linear(0.09f), quadratic(0.032f) {}
        
        // 带参数的构造函数
        PointLight(const glm::vec3& pos, const glm::vec3& col, float intens, float c, float l, float q)
            : position(pos), color(col), intensity(intens), constant(c), linear(l), quadratic(q) {}
    };

    // 材质结构体（用于Blinn-Phong光照）
    struct Material {
        glm::vec3 ambient;
        glm::vec3 specular;
        float shininess;
        bool useTextureDiffuse1;  // 是否使用texture_diffuse1（模型）还是material.texture_diffuse（地面）
        
        // 默认构造函数
        Material() : ambient(0.0f), specular(0.0f), shininess(32.0f), useTextureDiffuse1(false) {}
        
        // 带参数的构造函数
        Material(const glm::vec3& amb, const glm::vec3& spec, float shin, bool useTex)
            : ambient(amb), specular(spec), shininess(shin), useTextureDiffuse1(useTex) {}
    };

    // 场景数据缓存结构
    struct SceneData {
        glm::vec3 viewPos;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    // 【核心修改】使用静态方法获取静态数据
    // 这种写法兼容性最好，解决了 "无法将非静态数据成员声明为 inline" 和 "非静态成员引用" 的问题
    static SceneData& GetSceneData() {
        static SceneData data; // 静态局部变量，全程序只有一份
        return data;
    }

    // 初始化
    static void Init() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // 【核心修改】BeginScene 现在需要显式传入投影矩阵
    // 因为 Camera 类里没有投影矩阵，它在 Projection 类里
    static void BeginScene(const Camera& camera, const glm::mat4& projMatrix, Shader& shader) {
        shader.use();

        // 获取引用并更新数据
        SceneData& data = GetSceneData();

        data.viewPos = camera.getPos();
        data.viewMatrix = camera.getView();
        data.projectionMatrix = projMatrix;

        // 设置全局 Uniforms
        shader.setMat4("view", data.viewMatrix);
        shader.setMat4("projection", data.projectionMatrix);
        shader.setVec3("viewPos", data.viewPos);
    }

    static void EndScene() {
        // 可以在这里做一些清理工作
    }

    // 设置Blinn-Phong光照的点光源
    static void SetBlinnPhongLights(Shader& shader, 
                                     const std::vector<PointLight>& lights,
                                     float globalIntensity = 1.0f) {
        shader.use();
        int numLights = static_cast<int>(std::min(lights.size(), size_t(4)));  // shader最多支持4个光源
        shader.setInt("numLights", numLights);
        
        for (int i = 0; i < numLights; i++) {
            std::string prefix = "pointLights[" + std::to_string(i) + "]";
            shader.setVec3(prefix + ".position", lights[i].position);
            shader.setVec3(prefix + ".color", lights[i].color);
            shader.setFloat(prefix + ".intensity", lights[i].intensity * globalIntensity);
            shader.setFloat(prefix + ".constant", lights[i].constant);
            shader.setFloat(prefix + ".linear", lights[i].linear);
            shader.setFloat(prefix + ".quadratic", lights[i].quadratic);
        }
    }

    // 设置Blinn-Phong光照的材质（用于地面）
    static void SetBlinnPhongMaterial(Shader& shader, const Material& material) {
        shader.use();
        shader.setInt("material.texture_diffuse", 0);
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);
        shader.setBool("hasTextureDiffuse1", material.useTextureDiffuse1);
        
        // 设置默认值（地面使用）
        if (!material.useTextureDiffuse1) {
            shader.setVec4("materialColor", 1.0f, 1.0f, 1.0f, 1.0f);
            shader.setBool("hasTexture", false);
            shader.setVec3("emissiveColor", 0.0f, 0.0f, 0.0f);
            shader.setBool("hasEmissiveMap", false);
        }
    }

    // 设置Blinn-Phong光照的材质（用于模型，mesh会自动设置texture_diffuse1等）
    static void SetBlinnPhongMaterialForModel(Shader& shader, const Material& material) {
        shader.use();
        shader.setInt("material.texture_diffuse", 0);  // 备用
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);
        shader.setBool("hasTextureDiffuse1", true);  // 模型使用texture_diffuse1
    }

    // 提交绘制命令
    static void Submit(Shader& shader, ModelBase* model, const glm::mat4& transform, const std::vector<glm::mat4>& boneMatrices = {}) {
        if (!model) return;

        shader.use();
        shader.setMat4("model", transform);

        // 获取场景数据（用于透明排序等）
        SceneData& data = GetSceneData();

        // 判断模型类型并分发逻辑
        if (model->IsAnimated()) {
            // 安全转型
            auto animModel = static_cast<ModelAnimated*>(model);

            if (!boneMatrices.empty()) {
                // 路径 A: 有骨骼数据的动画绘制
                animModel->Draw(shader, boneMatrices);
            }
            else {
                // 路径 B: 无骨骼数据（默认姿态）
                animModel->Draw(shader, data.viewPos); // 注意：ModelAnimated::Draw 继承自 ModelBase 的版本需要 camPos 吗？
                // 如果 ModelAnimated::Draw(Shader&, vec3) 只是调用了 ModelAnimData::DrawMeshes，那么这是安全的
                // 根据之前的代码，这个重载是存在的
            }
        }
        else {
            // 路径 C: 静态模型 (需要 viewPos 进行透明物体排序)
            model->Draw(shader, data.viewPos);
        }
    }
};