#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1; // 对应 _d 贴图
uniform sampler2D texture_height1;  // 你的类将 aiTextureType_AMBIENT 映射到了这里 (AO 贴图)

void main()
{    
    // 1. 采样漫反射颜色
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 2. 采样环境光遮蔽 (AO)
    // 根据你的 Model 类逻辑，AO 加载到了 texture_height1
    float ao = texture(texture_height1, TexCoords).r;
    
    // 如果没有 AO 贴图，采样值可能是 0，这里做一个保护防止变全黑
    if(ao < 0.01) ao = 1.0; 

    // 3. 合成最终颜色：漫反射 * AO
    vec3 finalColor = texColor.rgb * ao;

    // 4. 强制 Alpha 为 1.0，彻底杜绝空洞
    FragColor = vec4(finalColor, 1.0);
}