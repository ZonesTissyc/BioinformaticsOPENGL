#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <custom/shader.h>

#include <string>
#include <vector>
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

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    // [新增] 存储材质颜色 (RGBA)
    glm::vec4 Color;

    glm::vec3 EmissiveColor;

    // [修改] 构造函数：增加了 color 参数，默认值为白色
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, glm::vec4 color = glm::vec4(1.0f), glm::vec3 emissive = glm::vec3(0.0f))
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->Color = color; // [新增] 保存颜色
        this->EmissiveColor = emissive;

        setupMesh();
    }

    // render the mesh
    void Draw(Shader& shader)
    {
        // 确保 shader 被 use（你的 shader.setXXX 可能已经做了 use()）
        shader.use();

        // ---- 查找特定类型的贴图，分配 texture unit ----
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int emissiveNr = 1;

        bool hasDiffuseMap = false;
        bool hasEmissiveMap = false;
        int emissiveMapUnit = -1;

        // 绑定纹理到连续的 texture units（从 0 开始）
        for (unsigned int i = 0; i < textures.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i].type;

            if (name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
                hasDiffuseMap = true;
            }
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);
            else if (name == "texture_emissive") {
                number = std::to_string(emissiveNr++);
                hasEmissiveMap = true;
                emissiveMapUnit = i; // 保存 texture unit
            }

            // 设置 sampler uniform，注意 uniform 名称必须和 shader 中一致
            string uniformName = name + number;
            glUniform1i(glGetUniformLocation(shader.ID, uniformName.c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 传递 materialColor：始终传（无论是否有 diffuse map）
        shader.setVec4("materialColor", Color);

        // hasTexture 语义改为 hasDiffuseMap
        shader.setBool("hasTexture", hasDiffuseMap);

        // 传 emissive 相关 uniforms
        shader.setVec3("emissiveColor", EmissiveColor);
        shader.setBool("hasEmissiveMap", hasEmissiveMap);
        if (hasEmissiveMap && emissiveMapUnit >= 0) {
            // 将 sampler 指向对应 unit（也可用 glUniform1i 但这里用 helper）
            shader.setInt("texture_emissive1", emissiveMapUnit);
        }
        else {
            // 如果没有 emissive map，确保 uniform 有个合理默认（可选）
            shader.setInt("texture_emissive1", 0);
        }

        // draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 重置 active 到 0
        glActiveTexture(GL_TEXTURE0);
    }

    bool isTransparent() const
    {
        // 1) 明确的 alpha
        if (Color.a < 0.999f) return true;

        // 2) 如果有 diffuse 纹理，检查文件名是否很可能带 alpha（png/tga/webp/tiff），或文件名包含 alpha/opacity 等关键字
        for (auto& tex : textures)
        {
            if (tex.type != "texture_diffuse") continue;
            string p = tex.path;
            // tolower
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return std::tolower(c); });

            if (p.find(".png") != string::npos ||
                p.find(".tga") != string::npos ||
                p.find(".webp") != string::npos ||
                p.find(".tiff") != string::npos ||
                p.find("alpha") != string::npos ||
                p.find("opacity") != string::npos)
                return true;
        }

        return false;
    }


private:
    unsigned int VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif