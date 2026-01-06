#version 330 core

//材质
uniform vec3 uKa;//环境光反射系数
uniform vec3 uKd;//漫反射系数
uniform vec3 uKs;//镜面反射系数
uniform float uShininess;//高光指数

//纹理
uniform sampler2D texture_diffuse1;
uniform bool hasTexture;
uniform vec4 materialColor;

//光源
uniform vec3 uIa;//光源的环境光强度
uniform vec3 uId;//漫反射强度
uniform vec3 uIs;//镜面反射强度

in vec3 vNormal;
in vec3 vLightDir;
in vec3 vViewDir;
in vec2 TexCoords;

void main()
{
    // 归一化
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vLightDir);
    vec3 V = normalize(vViewDir);
    
    
    
    // 获取基础颜色
    vec4 baseColor = materialColor;
    if (hasTexture) {
        baseColor = texture(texture_diffuse1, TexCoords);
    }
    
    // 环境光分量
    vec3 ambient = uKa * uIa * baseColor.rgb;
    
    // 漫反射分量
    float diff = max(dot(N, L), 0.0);//漫反射因子
    vec3 diffuse = uKd * uId * diff * baseColor.rgb;
    
    // 镜面反射分量（Blinn-Phong）
    vec3 H = normalize(L + V);//半程向量（光源方向与视线方向的中间向量）
    float spec = pow(max(dot(N, H), 0.0), uShininess);
    vec3 specular = uKs * uIs * spec;
    
    
    
    vec3 color = ambient + diffuse + specular;
    gl_FragColor = vec4(color, baseColor.a);
}