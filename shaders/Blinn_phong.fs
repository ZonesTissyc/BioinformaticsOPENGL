#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// 材质结构体
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// 点光源结构体
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

// 材质和光源定义
uniform Material material;
uniform PointLight pointLights[4];  // 支持最多4个点光源
uniform int numLights;              // 实际使用的光源数量
uniform vec3 viewPos;

// 计算单个点光源的Blinn-Phong光照
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // 光的方向和距离
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // 衰减计算
    float attenuation = 1.0 / (light.constant + 
                              light.linear * distance + 
                              light.quadratic * (distance * distance));
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * material.diffuse;
    
    // 镜面反射（Blinn-Phong核心）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    // 应用衰减并返回结果
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (diffuse + specular) * light.intensity;
}

void main() {
    // 标准化法线和视线方向
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 环境光（通常只计算一次）
    vec3 ambient = material.ambient * 0.1;  // 可根据需要调整
    
    // 初始化结果
    vec3 result = ambient;
    
    // 对每个点光源累加光照
    for(int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    // 输出最终颜色
    FragColor = vec4(result, 1.0);
    
    // 如果需要纹理支持，可以这样修改：
    // vec3 texColor = texture(material.diffuseMap, TexCoords).rgb;
    // FragColor = vec4(result * texColor, 1.0);
}