#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h> // 加载PBR

// #include <learnopengl/mesh.h>
#include "mesh_PBR.h"
#include <learnopengl/shader.h>
#include <stb/stb_image.h>

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace std;

// 声明：默认参数只放在这里
unsigned int TextureFromFile(const char* path, const string& directory, const aiScene* scene = nullptr, bool gamma = false);

class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    Model(const string& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // 修改 Model.h 内的 Draw()：
    void Draw(Shader& shader, const glm::vec3& camPos)
    {
        vector<Mesh*> opaqueMeshes;
        vector<pair<float, Mesh*>> transparentMeshes;

        for (auto& mesh : meshes)
        {
            if (mesh.isTransparent())
            {
                // 计算 mesh 中心
                glm::vec3 center(0.0f);
                for (auto& v : mesh.vertices)
                    center += v.Position;
                center /= (float)mesh.vertices.size();

                float dist = glm::length(camPos - center);
                transparentMeshes.push_back({ dist, &mesh });
            }
            else
            {
                opaqueMeshes.push_back(&mesh);
            }
        }

        // 1️⃣ 渲染不透明 Mesh
        for (auto& m : opaqueMeshes)
            m->Draw(shader);

        // 2️⃣ 渲染透明 Mesh，从远到近
        if (!transparentMeshes.empty())
        {
            std::sort(transparentMeshes.begin(), transparentMeshes.end(),
                [](const pair<float, Mesh*>& a, const pair<float, Mesh*>& b)
                { return a.first > b.first; }); // 距离越远越先渲染

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE); // 禁止写入深度缓存

            for (auto& p : transparentMeshes)
                p.second->Draw(shader);

            glDepthMask(GL_TRUE);  // 恢复深度写入
            glDisable(GL_BLEND);
        }
    }


private:
    void loadModel(const string& path)
    {
        Assimp::Importer importer;

        // 尝试多种 postprocess flag 的组合（你可以根据需要调整）
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_PreTransformVertices
        );

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        {
            cout << "[Model] ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        // 兼容 windows 路径
        size_t pos = path.find_last_of("/\\");
        if (pos == string::npos) directory = ".";
        else directory = path.substr(0, pos);

        cout << "[Model] Loaded scene. meshes=" << scene->mNumMeshes
            << " materials=" << scene->mNumMaterials
            << " nodes (root has children)=" << scene->mRootNode->mNumChildren
            << " directory=" << directory << endl;

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        if (!node) return;
        // 处理当前 node 的所有 mesh
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            unsigned int meshIndex = node->mMeshes[i];
            if (meshIndex >= scene->mNumMeshes)
            {
                cout << "[Model] Warning: node refers to invalid mesh index " << meshIndex << endl;
                continue;
            }
            aiMesh* mesh = scene->mMeshes[meshIndex];
            if (!mesh)
            {
                cout << "[Model] Warning: scene->mMeshes[" << meshIndex << "] == nullptr\n";
                continue;
            }
            cout << "[Model] processNode: processing mesh index=" << meshIndex << " numVerts=" << mesh->mNumVertices << " numFaces=" << mesh->mNumFaces << endl;
            meshes.push_back(processMesh(mesh, scene));
            cout << "[Model] push mesh index " << meshIndex << " -> total meshes: " << meshes.size() << endl;
        }

        // 递归子节点
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 1) 顶点
        vertices.reserve(mesh->mNumVertices ? mesh->mNumVertices : 0);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // 初始化，避免未定义行为
            vertex.Position = glm::vec3(0.0f);
            vertex.Normal = glm::vec3(0.0f);
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertex.Tangent = glm::vec3(0.0f);
            vertex.Bitangent = glm::vec3(0.0f);
            for (int b = 0; b < MAX_BONE_INFLUENCE; ++b) {
                vertex.m_BoneIDs[b] = -1;
                vertex.m_Weights[b] = 0.0f;
            }

            // Position (确保 mesh->mVertices 存在)
            if (mesh->mVertices)
            {
                vertex.Position = glm::vec3(
                    mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z
                );
            }

            // Normal
            if (mesh->HasNormals() && mesh->mNormals)
            {
                vertex.Normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                );
            }

            // TexCoords (UV) + Tangent/Bitangent：先判空再访问
            if (mesh->mTextureCoords && mesh->mTextureCoords[0])
            {
                // 有 UV
                vertex.TexCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );

                // 切线/副切线可能为空
                if (mesh->mTangents && mesh->mBitangents)
                {
                    vertex.Tangent = glm::vec3(
                        mesh->mTangents[i].x,
                        mesh->mTangents[i].y,
                        mesh->mTangents[i].z
                    );
                    vertex.Bitangent = glm::vec3(
                        mesh->mBitangents[i].x,
                        mesh->mBitangents[i].y,
                        mesh->mBitangents[i].z
                    );
                }
                else
                {
                    // 没有切线，保持为 0
                }
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // 2) 索引（faces）
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            if (face.mNumIndices == 0) continue;
            // 通常我们期望三角形（aiProcess_Triangulate 已处理），如果不是三角形可选择跳过或处理
            if (face.mNumIndices != 3)
            {
                // 记录并继续（安全起见）
                // 你也可以把非三角形转成三角形
                // cout << "[Model] Warning: face " << i << " has " << face.mNumIndices << " indices, skipping\n";
            }
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // 3) 材质 & 纹理
        aiMaterial* material = nullptr;
        if (scene && mesh->mMaterialIndex < scene->mNumMaterials)
            material = scene->mMaterials[mesh->mMaterialIndex];

        if (material)
        {
            // 加载 PBR 纹理（适用于 GLB/glTF 文件）
            // 对于 glTF，Assimp 会将纹理映射到特定的 aiTextureType
            
            // 1. Base Color (Albedo/Diffuse) - glTF 通常映射到 DIFFUSE
            // 尝试多种可能的类型
            auto baseColorMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_basecolor", scene);
            // 某些 glTF 可能使用 BASE_COLOR（如果 Assimp 版本支持）
            #ifdef aiTextureType_BASE_COLOR
            if (baseColorMaps.empty()) {
                baseColorMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_basecolor", scene);
            }
            #endif
            textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());

            // 2. Metallic-Roughness - glTF PBR (关键纹理)
            // glTF 的 Metallic-Roughness 可能映射到 UNKNOWN 或特定的类型
            auto mrMaps = loadMaterialTextures(material, aiTextureType_UNKNOWN, "texture_metallicroughness", scene);
            // 某些版本可能使用 METALLIC_ROUGHNESS（如果支持）
            #ifdef aiTextureType_METALLIC_ROUGHNESS
            if (mrMaps.empty()) {
                mrMaps = loadMaterialTextures(material, aiTextureType_METALLIC_ROUGHNESS, "texture_metallicroughness", scene);
            }
            #endif
            textures.insert(textures.end(), mrMaps.begin(), mrMaps.end());

            // 3. Normal Map - 尝试多种类型
            auto normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
            if (normalMaps.empty()) {
                // 某些格式用 HEIGHT 表示 normal
                normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
            }
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

            // 4. Occlusion (AO) - glTF
            auto aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao", scene);
            textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

            // 5. Emissive - glTF
            auto emissiveMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emissive", scene);
            textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());

            // 6. 传统纹理类型（兼容性，某些模型可能仍使用这些）
            auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            auto heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", scene);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }
        else
        {
            cout << "[Model] Warning: mesh.materialIndex invalid or material == nullptr\n";
        }

        // 4) 读取材质颜色（可选）
        glm::vec4 meshColor(1.0f);
        if (material)
        {
            aiColor4D diffuse(1.0f, 1.0f, 1.0f, 1.0f);
            if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            {
                meshColor = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
            }
            float opacity = 1.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity))
                meshColor.a = opacity;
        }

        cout << "[Model] processMesh: vertices=" << vertices.size()
            << ", indices=" << indices.size()
            << ", textures=" << textures.size()
            << ", color rgba(" << meshColor.r << "," << meshColor.g << "," << meshColor.b << "," << meshColor.a << ")\n";
        
        // 输出纹理详情
        for (size_t i = 0; i < textures.size(); i++) {
            cout << "[Model]   Texture[" << i << "]: type='" << textures[i].type 
                 << "', path='" << textures[i].path << "', id=" << textures[i].id << "\n";
        }

        // Mesh 构造函数是 Mesh(vertices, indices, textures, color)

        glm::vec3 emissiveColor(0.0f);
        if (material)
        {
            aiColor3D e(0.0f, 0.0f, 0.0f);
            if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, e))
            {
                emissiveColor = glm::vec3(e.r, e.g, e.b);
            }
        }

        // 在 processMesh 中，读取默认 factor
        float metallicFactor = 0.0f;
        float roughnessFactor = 1.0f;
        aiColor4D baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f);

        if (material) {
            // 这些宏由 assimp/pbrmaterial.h 定义
            material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor);
            material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor);
            material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColorFactor);
            
            cout << "[Model] PBR Material factors: baseColor=(" << baseColorFactor.r << "," << baseColorFactor.g << "," << baseColorFactor.b << "," << baseColorFactor.a 
                 << "), metallic=" << metallicFactor << ", roughness=" << roughnessFactor << "\n";
        } else {
            cout << "[Model] Warning: No material found, using default PBR factors\n";
        }

        return Mesh(
            vertices,
            indices,
            textures,
            meshColor,                                // glm::vec4 color
            emissiveColor,                            // glm::vec3 emissive
            glm::vec3(baseColorFactor.r, baseColorFactor.g, baseColorFactor.b), // baseColor
            metallicFactor,
            roughnessFactor
        );
    }


    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene)
    {
        vector<Texture> textures;
        if (!mat) return textures;

        unsigned int count = mat->GetTextureCount(type);
        cout << "[Model] loadMaterialTextures: type=" << (int)type << " count=" << count << " (" << typeName << ")\n";

        for (unsigned int i = 0; i < count; i++)
        {
            aiString str;
            // 使用简化版本的 GetTexture，只获取路径
            if (mat->GetTexture(type, i, &str) == AI_SUCCESS)
            {
                // 查重（基于路径字符串）
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
                    Texture texture;
                    cout << "[Model] Loading texture: path='" << str.C_Str() << "', type='" << typeName << "'\n";
                    texture.id = TextureFromFile(str.C_Str(), this->directory, scene, gammaCorrection);
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    
                    if (texture.id == 0) {
                        cout << "[Model] ERROR: Texture loading failed for '" << str.C_Str() << "' (type=" << typeName << "), texture ID is 0\n";
                    } else {
                        cout << "[Model] Texture loaded successfully: " << str.C_Str() << " -> id=" << texture.id << " (type=" << typeName << ")\n";
                    }
                    
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                }
                else
                {
                    cout << "[Model] Texture already loaded (cached): " << str.C_Str() << " (type=" << typeName << ")\n";
                }
            }
        }
        return textures;
    }
};

// 纹理加载函数（改进：统一强制到 RGBA，处理嵌入和磁盘图像，带日志）
unsigned int TextureFromFile(const char* path, const string& directory, const aiScene* scene, bool gamma)
{
    string filename = string(path);
    // 某些 path 可能以 "*index" 形式（嵌入）, 对 disk path 做合并
    string fullpath = filename;
    if (!filename.empty() && filename[0] != '*') {
        fullpath = directory + "/" + filename;
    }

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, nrComponents = 0;
    unsigned char* data = nullptr;
    bool needFree = false;

    // 处理嵌入纹理（Assimp 的 *index 形式或者名字）
    if (scene)
    {
        const aiTexture* aiTex = scene->GetEmbeddedTexture(path);
        
        // 如果直接查找失败，尝试处理 "*N" 格式（N 是索引）
        if (!aiTex && !filename.empty() && filename[0] == '*')
        {
            // 尝试解析 "*N" 格式
            try {
                int index = std::stoi(filename.substr(1));
                if (index >= 0 && index < static_cast<int>(scene->mNumTextures))
                {
                    aiTex = scene->mTextures[index];
                }
            }
            catch (...) {
                // 解析失败，继续尝试其他方式
            }
        }
        
        // 如果还是找不到，尝试按名称查找所有嵌入纹理
        if (!aiTex && scene->mNumTextures > 0)
        {
            for (unsigned int i = 0; i < scene->mNumTextures; i++)
            {
                if (scene->mTextures[i] && scene->mTextures[i]->mFilename.length > 0)
                {
                    if (strcmp(scene->mTextures[i]->mFilename.C_Str(), path) == 0 ||
                        strcmp(scene->mTextures[i]->mFilename.C_Str(), filename.c_str()) == 0)
                    {
                        aiTex = scene->mTextures[i];
                        break;
                    }
                }
            }
        }
        
        if (aiTex) {
            if (aiTex->mHeight == 0)
            {
                // 压缩的 embedded (jpg/png inside binary)
                int req_comp = 4;
                data = stbi_load_from_memory(
                    reinterpret_cast<const unsigned char*>(aiTex->pcData),
                    static_cast<int>(aiTex->mWidth),
                    &width, &height, &nrComponents, req_comp
                );
                if (data) {
                    nrComponents = req_comp;
                    needFree = true;
                    cout << "[TextureFromFile] loaded compressed embedded texture from memory. bytes=" << aiTex->mWidth
                        << " decoded: " << width << "x" << height << " comps=" << nrComponents << endl;
                }
                else {
                    cout << "[TextureFromFile] stbi_load_from_memory failed for embedded compressed texture\n";
                }
            }
            else
            {
                // 非压缩（raw RGBA framebuffer-like）
                data = reinterpret_cast<unsigned char*>(aiTex->pcData);
                width = aiTex->mWidth;
                height = aiTex->mHeight;
                nrComponents = 4; // Assimp 文档：非压缩通常为 RGBA
                needFree = false;
                cout << "[TextureFromFile] using raw embedded texture data. size=" << width << "x" << height << " comps=" << nrComponents << endl;
            }
        }
    }

    // 如果不是嵌入或嵌入加载失败，尝试文件系统加载
    if (!data)
    {
        int req_comp = 4;
        data = stbi_load(fullpath.c_str(), &width, &height, &nrComponents, req_comp);
        if (data) {
            nrComponents = req_comp;
            needFree = true;
            cout << "[TextureFromFile] loaded texture from disk: " << fullpath << " size=" << width << "x" << height << " comps=" << nrComponents << endl;
        }
        else {
            cout << "[TextureFromFile] failed to load texture from disk: " << fullpath << " stbi reason: " << stbi_failure_reason() << endl;
        }
    }

    if (data)
    {
        GLenum format = GL_RGBA;
        if (nrComponents == 4) format = GL_RGBA;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 2) format = GL_RG;
        else if (nrComponents == 1) format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, textureID);
        // 内存数据现在保证是 req_comp=4（若使用 stbi），或者 Assimp raw（也可能是 4）
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (needFree) stbi_image_free(data);

        cout << "[TextureFromFile] texture uploaded to GPU id=" << textureID << " size=(" << width << "x" << height << ") comps=" << nrComponents << endl;
    }
    else
    {
        cout << "[TextureFromFile] Texture failed to load at path: " << path << endl;
    }

    return textureID;
}

#endif
