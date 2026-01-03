#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>

#include <string>
#include <vector>
#include <ranges>
#include <algorithm> // 这里必须包含
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id = 0;
    string type;
    string path;
};

// ====== 替换 Mesh 类（只替换 class Mesh 部分） ======
class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    // 原来的
    glm::vec4 Color;
    glm::vec3 EmissiveColor;

    // PBR members (glTF metallic-roughness)
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor = 0.0f;
    float roughnessFactor = 1.0f;

    enum class AlphaMode { Opaque, Mask, Blend };
    AlphaMode alphaMode = AlphaMode::Opaque;
    float alphaCutoff = 0.5f;

    bool hasBaseColorMap = false;
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;
    bool hasMetallicMap = false;
    bool hasRoughnessMap = false;
    bool hasAOMap = false;
    bool hasEmissiveMap = false;

    // 构造函数：新增 pbr 参数（可向后兼容）
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
        glm::vec4 color, glm::vec3 emissive, glm::vec4 baseColor, float metallic, float roughness,
        AlphaMode alphaMode = AlphaMode::Opaque, float alphaCutoff = 0.5f)
    {
        this->vertices = std::move(vertices);
        this->indices = std::move(indices);
        this->textures = std::move(textures);
        this->Color = color;
        this->EmissiveColor = emissive;

        this->baseColorFactor = baseColor;
        this->metallicFactor = metallic;
        this->roughnessFactor = roughness;
        this->alphaMode = alphaMode;
        this->alphaCutoff = alphaCutoff;

        hasBaseColorMap = false;
        hasNormalMap = false;
        hasMetallicRoughnessMap = false;
        hasMetallicMap = false;
        hasRoughnessMap = false;
        hasAOMap = false;
        hasEmissiveMap = false;

        for (const auto& t : textures) {
            string p = t.type;
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            if (p.find("basecolor") != string::npos || p.find("base_color") != string::npos || p.find("base") != string::npos || p.find("diffuse") != string::npos) hasBaseColorMap = true;
            else if (p.find("normal") != string::npos) hasNormalMap = true;
            else if (p.find("metallicroughness") != string::npos) hasMetallicRoughnessMap = true;
            else if (p.find("metallic") != string::npos) hasMetallicMap = true;
            else if (p.find("roughness") != string::npos) hasRoughnessMap = true;
            else if (p.find("ao") != string::npos || p.find("ambient") != string::npos) hasAOMap = true;
            else if (p.find("emissive") != string::npos) hasEmissiveMap = true;
        }

        setupMesh();
    }


    // render the mesh — 绑定 PBR 纹理 & uniforms
    void Draw(Shader& shader)
    {
        shader.use();

        // 查询 GPU 可以使用的最大 texture units（防止越界）
        GLint maxUnits = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);

        int unit = 0;
        auto bindIfPresent = [&](const char* uniformName, const string& findKey1, const string& findKey2 = "") {
            if (unit >= maxUnits) return; // no more units
            unsigned int tid = findTextureIdByType(findKey1, findKey2);
            if (tid != 0) {
                shader.setInt(uniformName, unit);
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, tid);
                ++unit;
            }
            };

        // 只有在确实存在对应纹理 id 时才绑定
        if (hasBaseColorMap) bindIfPresent("baseColorMap", "base", "diffuse");
        if (hasNormalMap) bindIfPresent("normalMap", "normal");
        if (hasMetallicRoughnessMap) bindIfPresent("metallicRoughnessMap", "metallicroughness");
        if (!hasMetallicRoughnessMap && hasMetallicMap) bindIfPresent("metallicMap", "metallic");
        if (!hasMetallicRoughnessMap && hasRoughnessMap) bindIfPresent("roughnessMap", "roughness");
        if (hasAOMap) bindIfPresent("aoMap", "ao", "ambient");
        if (hasEmissiveMap) bindIfPresent("emissiveMap", "emissive");

        // 常量 uniforms
        shader.setVec4("baseColorFactor", baseColorFactor);
        shader.setFloat("metallicFactor", metallicFactor);
        shader.setFloat("roughnessFactor", roughnessFactor);
        shader.setVec3("emissiveFactor", EmissiveColor);
        shader.setFloat("alphaCutoff", alphaCutoff);

        // 标志 uniforms
        shader.setBool("hasBaseColorMap", hasBaseColorMap && findTextureIdByType("base", "diffuse") != 0);
        shader.setBool("hasNormalMap", hasNormalMap && findTextureIdByType("normal") != 0);
        shader.setBool("hasMetallicRoughnessMap", hasMetallicRoughnessMap && findTextureIdByType("metallicroughness") != 0);
        shader.setBool("hasMetallicMap", (!hasMetallicRoughnessMap) && hasMetallicMap && findTextureIdByType("metallic") != 0);
        shader.setBool("hasRoughnessMap", (!hasMetallicRoughnessMap) && hasRoughnessMap && findTextureIdByType("roughness") != 0);
        shader.setBool("hasAOMap", hasAOMap && findTextureIdByType("ao", "ambient") != 0);
        shader.setBool("hasEmissiveMap", hasEmissiveMap && findTextureIdByType("emissive") != 0);
        shader.setBool("alphaModeBlend", alphaMode == AlphaMode::Blend);
        shader.setBool("alphaModeMask", alphaMode == AlphaMode::Mask);

        // draw safety checks
        if (VAO == 0) {
            // 防护：VAO 未初始化（会导致崩溃）
            cout << "[Mesh::Draw] Warning: VAO == 0, skipping draw\n";
            return;
        }
        if (indices.empty()) {
            cout << "[Mesh::Draw] Warning: no indices, skipping draw\n";
            return;
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 恢复
        glActiveTexture(GL_TEXTURE0);
    }


    // 简单工具：按 type 关键字找 texture.id（优先第一个匹配）
    unsigned int findTextureIdByType(const string& key1, const string& key2 = "") const {
        for (auto& t : textures) {
            string p = t.type;
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return std::tolower(c); });
            if (!key1.empty() && p.find(key1) != string::npos) return t.id;
            if (!key2.empty() && p.find(key2) != string::npos) return t.id;
        }
        return 0;
    }

    // isTransparent 保持或改进（略）
    bool isTransparent() const {
        if (alphaMode == AlphaMode::Blend) return true;
        if (Color.a < 0.999f) return true;
        for (auto& tex : textures) {
            string p = tex.path;
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return std::tolower(c); });
            if (p.find(".png") != string::npos || p.find(".tga") != string::npos || p.find("alpha") != string::npos) return true;
        }
        return false;
    }

private:
    unsigned int VBO, EBO;
    // setupMesh 保持不变（如你已有）

    // 放在 Mesh 类内部：安全的 setupMesh 实现
    void setupMesh()
    {
        // 生成/初始化
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        if (!vertices.empty())
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        else
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        if (!indices.empty())
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        else
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // texcoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // bone ids (integer)
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
    }

};


#endif