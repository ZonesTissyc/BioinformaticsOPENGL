#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

// 纹理
uniform sampler2D texture_diffuse1;
uniform bool hasTexture;
uniform vec4 materialColor;

// 光照
uniform vec3 lightPos;    // 点光源
uniform vec3 lightColor;  
uniform vec3 viewPos;    


uniform vec3 materialSpecular;  
uniform float materialShininess;// 高光指数


uniform vec3 emissiveColor;
uniform bool hasEmissiveMap;
uniform sampler2D texture_emissive1;

void main()
{
 //纹理   
    vec4 baseColor;
    if(hasTexture) {
        baseColor = texture(texture_diffuse1, TexCoords) * materialColor;
    } else {
        baseColor = materialColor;
    }
  
    if(baseColor.a < 0.1) discard;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 reflectDir = reflect(-lightDir, norm);

//环境光
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
//漫反射
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
//镜面反射
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = spec * materialSpecular * lightColor;

//混合
    vec3 finalColor = (ambient + diffuse + specular) * baseColor.rgb ;
    FragColor = vec4(finalColor, baseColor.a);
}