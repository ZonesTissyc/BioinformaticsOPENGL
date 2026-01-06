#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 vNormal;
out vec3 FragPos; //顶点位置
out vec3 vViewDir;//视线方向
out vec3 vLightDir;//光源的方向

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 uLightPos;//光源的位置

void main()
{
    TexCoords = aTexCoords;  //纹理坐标  
    // 顶点变换到观察空间
    FragPos = vec3(model * vec4(aPos, 1.0)); 

    // 法线变换到观察空间(法线的逆的转置)
    vNormal = mat3(transpose(inverse(model))) * aNormal;  

    // 计算观察空间中的光线方向（点光源）
    vLightDir = uLightPos - FragPos;

    vViewDir = -FragPos;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
}