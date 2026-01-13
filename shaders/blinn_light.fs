#version 330 core

//材质
uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

//纹理
uniform sampler2D texture_diffuse1;
uniform bool hasTexture;
uniform vec4 materialColor;

//光源
uniform vec3 uIa;
uniform vec3 uId;
uniform vec3 uIs;

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
 
    vec4 baseColor = materialColor;
    if (hasTexture) {
        baseColor = texture(texture_diffuse1, TexCoords);
    }
    
    vec3 ambient = uKa * uIa * baseColor.rgb;
    
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = uKd * uId * diff * baseColor.rgb;
    
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uShininess);
    vec3 specular = uKs * uIs * spec;
    
    vec3 color = ambient + diffuse + specular;
    gl_FragColor = vec4(color, baseColor.a);
}