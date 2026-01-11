#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <custom/mesh.h>
#include <custom/shader.h>
#include <custom/assimp_glm_helpers.h>
#include <custom/animdata.h> // BoneInfo definition

#include <string>
#include <iostream>
#include <map>
#include <vector>

class ModelAnimData
{
public:
    ModelAnimData(const std::string& path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // 资源级绘制（不上传骨骼矩阵，实例类负责上传）
    void DrawMeshes(Shader& shader)
    {
        for (auto& mesh : meshes)
            mesh.Draw(shader);
    }

    // 供 Animation 使用 / 查询
    std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    const std::string& GetDirectory() const { return directory; }
    const std::vector<Mesh>& GetMeshes() const { return meshes; }

	// 调试用：打印头的骨信息
    void DebugPrintHeadBone(const std::string& headName)
    {
        auto it = m_BoneInfoMap.find(headName);
        if (it == m_BoneInfoMap.end())
        {
            std::cout << "[AnimData] Head bone NOT found: "
                << headName << std::endl;
            return;
        }

        const BoneInfo& info = it->second;

        std::cout << "[AnimData] Head bone found!\n";
        std::cout << "  name   : " << headName << "\n";
        std::cout << "  id     : " << info.id << "\n";
        std::cout << "  offset : \n";

        const glm::mat4& m = info.offset;
        for (int r = 0; r < 4; ++r)
        {
            std::cout << "    ";
            for (int c = 0; c < 4; ++c)
                std::cout << m[r][c] << " ";
            std::cout << "\n";
        }
    }

private:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection{ false };

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // ---------------- Model loading ----------------

    void loadModel(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_LimitBoneWeights
        );

        if (!scene || !scene->mRootNode)
        {
            std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
            return;
        }

        size_t pos = path.find_last_of("/\\");
        directory = (pos == std::string::npos) ? "" : path.substr(0, pos);

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        if (!node) return;

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            processNode(node->mChildren[i], scene);
    }

    // ---------------- Mesh processing ----------------

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        vertices.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex vertex{};                     // 全量零初始化
            SetVertexBoneDataToDefault(vertex);  // bone defaults

            // position
            vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);

            // normal
            if (mesh->HasNormals())
                vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

            // texcoords
            if (mesh->HasTextureCoords(0))
            {
                vertex.TexCoords = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            // tangent / bitangent
            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
                vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);
            }
            else
            {
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        // materials
        aiMaterial* material = nullptr;
        if (scene && mesh->mMaterialIndex < scene->mNumMaterials)
            material = scene->mMaterials[mesh->mMaterialIndex];

        if (material)
        {
            auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
            if (diffuseMaps.empty())
                diffuseMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_diffuse", scene);

            auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
            auto normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
            auto heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", scene);

            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }

        ExtractBoneWeightForVertices(vertices, mesh);

        return Mesh(vertices, indices, textures);
    }

    // ---------------- Bone handling ----------------

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_BoneIDs[i] = boneID;
                vertex.m_Weights[i] = weight;
                return;
            }
        }
    }

    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh)
    {
        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            int boneID = -1;

            if (m_BoneInfoMap.count(boneName) == 0)
            {
                BoneInfo info;
                info.id = m_BoneCounter;
                info.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix
                );
                m_BoneInfoMap[boneName] = info;
                boneID = m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }

            const aiBone* bone = mesh->mBones[boneIndex];
            for (unsigned int w = 0; w < bone->mNumWeights; ++w)
            {
                int v = bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;

                if (v >= 0 && v < (int)vertices.size())
                    SetVertexBoneData(vertices[v], boneID, weight);
            }
        }
    }

    // ---------------- Texture loading ----------------

    unsigned int TextureFromFile(const char* path, const std::string& directory, const aiScene* scene)
    {
        std::string filename = path;
        if (!directory.empty() && filename[0] != '*')
            filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int w, h, comp;
        unsigned char* data = nullptr;
        bool freeData = false;

        if (scene)
        {
            if (const aiTexture* tex = scene->GetEmbeddedTexture(path))
            {
                if (tex->mHeight == 0)
                {
                    data = stbi_load_from_memory(
                        (unsigned char*)tex->pcData,
                        tex->mWidth,
                        &w, &h, &comp, 0
                    );
                    freeData = true;
                }
                else
                {
                    data = (unsigned char*)tex->pcData;
                    w = tex->mWidth;
                    h = tex->mHeight;
                    comp = 4;
                }
            }
        }

        if (!data)
        {
            data = stbi_load(filename.c_str(), &w, &h, &comp, 0);
            freeData = true;
        }

        if (data)
        {
            GLenum format = (comp == 4) ? GL_RGBA : (comp == 3) ? GL_RGB : GL_RED;
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (freeData) stbi_image_free(data);
        }

        return textureID;
    }

    std::vector<Texture> loadMaterialTextures(
        aiMaterial* mat,
        aiTextureType type,
        std::string typeName,
        const aiScene* scene)
    {
        std::vector<Texture> textures;

        if (!mat) return textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (auto& t : textures_loaded)
            {
                if (t.path == str.C_Str())
                {
                    textures.push_back(t);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                Texture tex;
                tex.id = TextureFromFile(str.C_Str(), directory, scene);
                tex.type = typeName;
                tex.path = str.C_Str();

                textures.push_back(tex);
                textures_loaded.push_back(tex);
            }
        }

        return textures;
    }
};
