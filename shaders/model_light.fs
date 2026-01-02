#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
// 假设有个简单的光源位置
uniform vec3 lightPos; 
uniform vec3 viewPos;

void main()
{    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 1. 环境光 (Ambient) - 保证即使背光处也有底色
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
    
    // 2. 漫反射 (Diffuse) - 简单的光照
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    // 结果合成
    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}