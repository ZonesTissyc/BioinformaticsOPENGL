#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// 材质
struct Material {
    sampler2D texture_diffuse;  
    vec3 ambient;              
    vec3 specular;             
    float shininess;           
};
uniform Material material;

// 点光源
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLights[4];  
uniform int numLights;             
uniform vec3 viewPos;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // 衰减
    float attenuation = 1.0 / (light.constant + 
                              light.linear * distance + 
                              light.quadratic * (distance * distance));

    vec3 diffuseColor = texture(material.texture_diffuse, texCoords).rgb;  
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * diffuseColor;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (diffuse + specular) * light.intensity;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 texColor = texture(material.texture_diffuse, TexCoords).rgb; 
    vec3 ambient = material.ambient * texColor;  
    
    vec3 result = ambient;
    
    for(int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, TexCoords);
    }
    
    FragColor = vec4(result, 1.0);
}