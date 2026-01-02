#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
// 注意：这里一定要和 mesh.h 里的 setupMesh 对应
layout (location = 5) in ivec4 aBoneIds; 
layout (location = 6) in vec4 aWeights;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    vec4 totalPosition = vec4(0.0f);
    
    // 检查是否是有效骨骼（防止静态模型没有权重导致消失）
    bool hasBone = false;
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
        if(aBoneIds[i] != -1 && aWeights[i] > 0.0) {
            hasBone = true;
            break;
        }
    }

    if (!hasBone) {
        // 如果没有骨骼权重，就像普通模型一样渲染
        totalPosition = vec4(aPos, 1.0f);
    } 
    else {
        // 骨骼变换计算
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(aBoneIds[i] == -1) 
                continue;
            
            if(aBoneIds[i] >= MAX_BONES) 
            {
                totalPosition = vec4(aPos,1.0f);
                break;
            }
            
            vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos,1.0f);
            totalPosition += localPosition * aWeights[i];
        }
    }

    gl_Position = projection * view * model * totalPosition;
    TexCoords = aTexCoords;
}