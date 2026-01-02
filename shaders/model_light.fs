#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec4 materialColor; 
uniform bool hasTexture;

uniform vec3 lightPos; 
uniform vec3 viewPos;

// Emission
uniform vec3 emissiveColor;
uniform bool hasEmissiveMap;
uniform sampler2D texture_emissive1;
// 可调节强度，便于不爆白
uniform float emissiveStrength; // 在 CPU 默认为 1.0

void main()
{
    // base color
    vec4 baseColor = hasTexture ? texture(texture_diffuse1, TexCoords) * materialColor : materialColor;

    if (baseColor.a < 0.01)
        discard;

    // simple lighting (ambient + diffuse)
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 lighting = ambient + diffuse;

    // emission
    vec3 emissiveTex = hasEmissiveMap ? texture(texture_emissive1, TexCoords).rgb : vec3(1.0);
    vec3 emissive = emissiveColor * emissiveTex * emissiveStrength;

    // 合并
    vec3 linearColor = lighting * baseColor.rgb + emissive;

    // 简单 tonemap（可防止亮度爆表）
    linearColor = linearColor / (linearColor + vec3(1.0));

    // gamma 校正（如果你想）
    // linearColor = pow(linearColor, vec3(1.0/2.2));

    FragColor = vec4(linearColor, baseColor.a);
}
