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

void main()
{    
    // --- 1. 获取基础颜色 (Base Color) ---
    vec4 baseColor;
    if(hasTexture) {
        baseColor = texture(texture_diffuse1, TexCoords) * materialColor;
    } else {
        baseColor = materialColor;
    }

    if(baseColor.a < 0.1)
        discard;

    // --- 2. 光照计算 (Ambient + Diffuse) ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * vec3(1.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 lighting = (ambient + diffuse) * baseColor.rgb;

    // --- 3. Emission ---
    vec3 emission = emissiveColor;
    if(hasEmissiveMap)
    {
        emission = texture(texture_emissive1, TexCoords).rgb;
    }

    vec3 finalColor = lighting + emission;

    FragColor = vec4(finalColor, baseColor.a);
}
