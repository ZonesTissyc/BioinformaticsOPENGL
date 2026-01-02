#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp> // 必须包含 Importer
#include <functional>
#include <algorithm>
#include <string>

#include <learnopengl/bone.h>
#include "model_new.h" // 必须包含，因为需要访问 BoneInfo 和 Model 类

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    // 构造函数：加载动画文件，并利用 Model 中的骨骼映射信息
    Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        // 加载动画文件
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        // 简单的错误检查
        if (!scene || !scene->mRootNode) {
            std::cout << "ERROR::ANIMATION:: Could not read animation file: " << animationPath << std::endl;
            return;
        }

        // 获取第一个动画
        auto animation = scene->mAnimations[0];
        m_Duration = (float)animation->mDuration;
        m_TicksPerSecond = (float)animation->mTicksPerSecond;

        // 读取整个骨骼层级结构
        ReadHierarchyData(m_RootNode, scene->mRootNode);

        // 读取骨骼的关键帧数据
        ReadMissingBones(animation, *model);
    }

    ~Animation() {}

    // 根据名字查找 Bone 对象（用于获取当前的变换矩阵）
    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.GetBoneName() == name;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    // Getters
    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
    // 读取动画通道数据，并匹配 Model 中的骨骼 ID
    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;

        // 引用 Model 类中已经生成的骨骼 Map
        auto& boneInfoMap = model.GetBoneInfoMap();
        int& boneCount = model.GetBoneCount();

        // 遍历所有动画通道（每个通道对应一个骨骼的运动轨迹）
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            // 如果这个骨骼在 Model 加载 Mesh 时没有出现过（可能是纯控制点），则将其加入 Map
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = glm::mat4(1.0f); // 无 Offset，因为它不影响顶点
                boneInfoMap[boneName] = newBoneInfo;
                boneCount++;
            }

            // 创建 Bone 对象，存储关键帧
            m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[boneName].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    // 递归读取 Assimp 的节点层级
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    // 辅助函数：将 Assimp 矩阵转换为 GLM 矩阵
    // 放在这里是为了不依赖外部 helper 头文件，保证 animation.h 独立性
    glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        // Assimp: Row-major, GLM: Column-major
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
};

#endif