#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// 采样器名称需与类中 loadMaterialTextures 的 typeName 一致
uniform sampler2D texture_diffuse1; // 对应漫反射 (_d)
uniform sampler2D texture_height1;  // 你类中将 aiTextureType_AMBIENT 映射为了 texture_height (AO)

void main()
{   
    // 1. 获取基础颜色
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 2. 彻底解决空洞：
    // 很多 TGA/PNG 的 Alpha 通道在非填充区域是 0。
    // 如果你之前的代码有 if(texColor.a < 0.1) discard; 就会产生洞。
    // 对于不透明的士兵，我们直接【禁用】discard，或将阈值设得极低。
    if(texColor.a < 0.01) discard; 

    // 3. 获取环境光遮蔽 (AO)
    // 根据你的类代码，AO 贴图被加载到了 texture_height1
    // 为了防止缺失 AO 贴图导致模型变黑，我们做一个简单的亮度补偿
    float ao = texture(texture_height1, TexCoords).r;
    
    // 如果 ao 采样结果接近 0（可能没图），则强制设为 1.0 或给个基础亮度
    if (ao < 0.01) ao = 1.0; 

    // 4. 合成颜色
    vec3 finalRGB = texColor.rgb * ao;

    // 5. 强制 Alpha 为 1.0，确保没有任何透明孔洞
    FragColor = vec4(finalRGB, 1.0);
}