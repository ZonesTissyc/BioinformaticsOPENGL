#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// 材质结构体（包含漫反射纹理）
struct Material {
    sampler2D texture_diffuse;  // 漫反射纹理采样器（用于地面等简单对象）
    vec3 ambient;               // 环境光系数
    vec3 specular;              // 高光颜色
    float shininess;            // 高光指数
};
uniform Material material;

// 兼容 mesh 的纹理命名方式（texture_diffuse1, texture_diffuse2 等）
uniform sampler2D texture_diffuse1;  // 第一个漫反射纹理（用于模型）
uniform bool hasTextureDiffuse1;    // 是否使用 texture_diffuse1

// 材质颜色调制（mesh 会设置这个）
uniform vec4 materialColor;         // 材质颜色（RGBA），用于调制纹理
uniform bool hasTexture;            // 是否有纹理（mesh 会设置）

// Emission（自发光）
uniform vec3 emissiveColor;         // 自发光颜色
uniform bool hasEmissiveMap;        // 是否有自发光纹理
uniform sampler2D texture_emissive1; // 自发光纹理

// 点光源结构体
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

// 全局Uniform
uniform PointLight pointLights[4];  // 最多4个点光源
uniform int numLights;              // 实际使用的光源数量
uniform vec3 viewPos;

// 获取纹理颜色（兼容两种纹理命名方式，并应用materialColor调制）
vec3 getTextureColor(vec2 texCoords) {
    vec3 texColor;
    if (hasTextureDiffuse1) {
        // 使用 texture_diffuse1（模型使用）
        if (hasTexture) {
            // 有纹理：采样纹理并应用materialColor调制
            texColor = texture(texture_diffuse1, texCoords).rgb * materialColor.rgb;
        } else {
            // 没有纹理：直接使用materialColor
            texColor = materialColor.rgb;
        }
    } else {
        // 使用 material.texture_diffuse（地面使用）
        texColor = texture(material.texture_diffuse, texCoords).rgb;
    }
    return texColor;
}

// 获取自发光颜色
vec3 getEmissionColor(vec2 texCoords) {
    if (hasEmissiveMap) {
        return texture(texture_emissive1, texCoords).rgb;
    } else {
        return emissiveColor;
    }
}

// 计算单个点光源的Blinn-Phong光照（增加纹理坐标参数）
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords) {
    // 光的方向与距离
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // 衰减计算
    float attenuation = 1.0 / (light.constant + 
                              light.linear * distance + 
                              light.quadratic * (distance * distance));
    
    // 漫反射（使用纹理颜色）
    vec3 diffuseColor = getTextureColor(texCoords);  // 采样纹理颜色
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * diffuseColor;
    
    // 镜面反射（Blinn-Phong）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    // 应用衰减
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (diffuse + specular) * light.intensity;
}

void main() {
    // 标准化法线和视线方向
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 获取基础颜色（包含纹理和materialColor调制）
    vec3 texColor = getTextureColor(TexCoords);
    
    // Alpha测试（如果alpha太低则丢弃片段）
    float alpha = materialColor.a;
    if (hasTextureDiffuse1 && hasTexture) {
        alpha = texture(texture_diffuse1, TexCoords).a * materialColor.a;
    }
    if (alpha < 0.1) {
        discard;
    }
    
    // 环境光（结合纹理颜色）
    vec3 ambient = material.ambient * texColor;  // 环境光 = 材质环境光系数 * 纹理颜色
    
    // 初始化结果
    vec3 result = ambient;
    
    // 累加所有点光源的光照
    for(int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, TexCoords);
    }
    
    // 添加自发光（Emission）
    vec3 emission = getEmissionColor(TexCoords);
    result += emission;
    
    // 输出最终颜色（保留alpha）
    FragColor = vec4(result, alpha);
}