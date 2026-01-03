#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // 
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos; // 传递片段的世界坐标
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    // 计算世界空间坐标
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    // 法线矩阵处理（防止缩放导致法线变形）
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    Normal = N;

    // Tangent basis for normal mapping (safe even if tangents are zero)
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    // Orthonormalize T wrt N to avoid artifacts
    T = normalize(T - dot(T, N) * N);
    // If B is invalid, reconstruct
    if (length(B) < 1e-5) {
        B = normalize(cross(N, T));
    }
    TBN = mat3(T, B, N);
    
    gl_Position = projection * view * model * vec4(aPos, 1.0); // 
}