#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
// 新增：材质颜色 和 是否有纹理的开关
uniform vec4 materialColor; 
uniform bool hasTexture;

uniform vec3 lightPos; 
uniform vec3 viewPos;

void main()
{    
    // 1. 获取基础颜色 (Base Color)
    vec4 baseColor;
    if(hasTexture) {
        // 如果有纹理，采样纹理，并乘上材质颜色(通常是白色，如果有特定染色则是混合)
        baseColor = texture(texture_diffuse1, TexCoords) * materialColor;
    } else {
        // 如果没有纹理，直接使用材质颜色 (修复扶手黑色的问题)
        baseColor = materialColor;
    }

    // 如果完全透明，丢弃 (可选)
    if(baseColor.a < 0.1)
        discard;

    // 2. 光照计算 (Ambient + Diffuse)
    // 环境光
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    // 3. 最终颜色 = (环境光 + 漫反射) * 物体本身的颜色
    vec3 lighting = (ambient + diffuse);
    
    // 结果需保留原始的 alpha 值 (修复水面消失)
    FragColor = vec4(lighting * baseColor.rgb, baseColor.a);
}