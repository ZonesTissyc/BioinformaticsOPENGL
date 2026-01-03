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

    // PBR members
    glm::vec3 baseColorFactor = glm::vec3(1.0f);
    float metallicFactor = 0.0f;
    float roughnessFactor = 1.0f;

    bool hasBaseColorMap = false;
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;
    bool hasAOMap = false;
    bool hasEmissiveMap = false;

    // 构造函数：新增 pbr 参数（可向后兼容）
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
        glm::vec4 color, glm::vec3 emissive, glm::vec3 baseColor, float metallic, float roughness)
    {
        this->vertices = std::move(vertices);
        this->indices = std::move(indices);
        this->textures = std::move(textures);
        this->Color = color;
        this->EmissiveColor = emissive;

        this->baseColorFactor = baseColor;
        this->metallicFactor = metallic;
        this->roughnessFactor = roughness;

        hasBaseColorMap = false;
        hasNormalMap = false;
        hasMetallicRoughnessMap = false;
        hasAOMap = false;
        hasEmissiveMap = false;

        // 只有在纹理 ID 有效时才设置标志位
        for (const auto& t : textures) {
            if (t.id == 0) {
                cout << "[Mesh::Constructor] Warning: texture with type '" << t.type << "' has invalid ID (0), skipping\n";
                continue; // 跳过无效纹理
            }
            string p = t.type;
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            if (p.find("base") != string::npos || p.find("diffuse") != string::npos) {
                hasBaseColorMap = true;
                cout << "[Mesh::Constructor] Found baseColorMap texture: type='" << t.type << "', id=" << t.id << "\n";
            }
            else if (p.find("normal") != string::npos) {
                hasNormalMap = true;
                cout << "[Mesh::Constructor] Found normalMap texture: type='" << t.type << "', id=" << t.id << "\n";
            }
            else if (p.find("metallicroughness") != string::npos || p.find("metallic") != string::npos) {
                hasMetallicRoughnessMap = true;
                cout << "[Mesh::Constructor] Found metallicRoughnessMap texture: type='" << t.type << "', id=" << t.id << "\n";
            }
            else if (p.find("ao") != string::npos || p.find("ambient") != string::npos) {
                hasAOMap = true;
                cout << "[Mesh::Constructor] Found aoMap texture: type='" << t.type << "', id=" << t.id << "\n";
            }
            else if (p.find("emissive") != string::npos) {
                hasEmissiveMap = true;
                cout << "[Mesh::Constructor] Found emissiveMap texture: type='" << t.type << "', id=" << t.id << "\n";
            }
        }

        setupMesh();
    }


    // render the mesh — 绑定 PBR 纹理 & uniforms
    void Draw(Shader& shader)
    {
        cout << "[Mesh::Draw] Starting draw. textures count=" << textures.size() 
             << ", hasBaseColorMap=" << hasBaseColorMap 
             << ", baseColorFactor=(" << baseColorFactor.r << "," << baseColorFactor.g << "," << baseColorFactor.b << ")\n";
        
        // 输出所有纹理信息
        for (size_t i = 0; i < textures.size(); i++) {
            cout << "[Mesh::Draw]   Texture[" << i << "]: type='" << textures[i].type 
                 << "', path='" << textures[i].path << "', id=" << textures[i].id << "\n";
        }
        
        shader.use();

        // 查询 GPU 可以使用的最大 texture units（防止越界）
        GLint maxUnits = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);

        int unit = 0;
        bool baseColorBound = false;
        bool normalBound = false;
        bool metallicRoughnessBound = false;
        bool aoBound = false;
        bool emissiveBound = false;

        auto bindIfPresent = [&](const char* uniformName, const string& findKey1, const string& findKey2 = "", bool* boundFlag = nullptr) {
            if (unit >= maxUnits) {
                cout << "[Mesh::Draw] Warning: texture unit limit reached (" << maxUnits << "), skipping " << uniformName << "\n";
                return false;
            }
            unsigned int tid = findTextureIdByType(findKey1, findKey2);
            if (tid != 0) {
                shader.setInt(uniformName, unit);
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, tid);
                cout << "[Mesh::Draw] Bound texture " << uniformName << " to unit " << unit << " (id=" << tid << ")\n";
                ++unit;
                if (boundFlag) *boundFlag = true;
                return true;
            }
            else {
                cout << "[Mesh::Draw] Warning: texture " << uniformName << " not found or invalid (id=0)\n";
                return false;
            }
        };

        // 无论标志位如何，都尝试查找和绑定纹理（因为标志位可能没有正确设置）
        // 只有在纹理 ID 有效时才会实际绑定
        baseColorBound = bindIfPresent("baseColorMap", "base", "diffuse", &baseColorBound);
        normalBound = bindIfPresent("normalMap", "normal", "", &normalBound);
        metallicRoughnessBound = bindIfPresent("metallicRoughnessMap", "metallicroughness", "metallic", &metallicRoughnessBound);
        aoBound = bindIfPresent("aoMap", "ao", "ambient", &aoBound);
        emissiveBound = bindIfPresent("emissiveMap", "emissive", "", &emissiveBound);

        // 常量 uniforms
        shader.setVec3("baseColorFactor", baseColorFactor);
        shader.setFloat("metallicFactor", metallicFactor);
        shader.setFloat("roughnessFactor", roughnessFactor);
        shader.setVec3("emissiveFactor", EmissiveColor);

        // 标志 uniforms - 只有在纹理实际绑定成功时才设置为 true
        shader.setBool("hasBaseColorMap", baseColorBound);
        shader.setBool("hasNormalMap", normalBound);
        shader.setBool("hasMetallicRoughnessMap", metallicRoughnessBound);
        shader.setBool("hasAOMap", aoBound);
        shader.setBool("hasEmissiveMap", emissiveBound);

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
        cout << "[Mesh::findTextureIdByType] Searching for key1='" << key1 << "', key2='" << key2 << "'\n";
        for (auto& t : textures) {
            string p = t.type;
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return std::tolower(c); });
            cout << "[Mesh::findTextureIdByType]   Checking texture: type='" << t.type << "' (lowercase='" << p << "'), id=" << t.id << "\n";
            if (!key1.empty() && p.find(key1) != string::npos) {
                cout << "[Mesh::findTextureIdByType]   Match found with key1! Returning id=" << t.id << "\n";
                return t.id;
            }
            if (!key2.empty() && p.find(key2) != string::npos) {
                cout << "[Mesh::findTextureIdByType]   Match found with key2! Returning id=" << t.id << "\n";
                return t.id;
            }
        }
        cout << "[Mesh::findTextureIdByType]   No match found, returning 0\n";
        return 0;
    }

    // isTransparent 保持或改进（略）
    bool isTransparent() const {
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