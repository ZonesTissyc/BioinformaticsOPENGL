#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D baseColorMap;
uniform bool hasBaseColorMap;
uniform vec3 baseColorFactor;

void main() {
    vec3 col = baseColorFactor; // 默认使用 baseColorFactor
    
    // 只有在 hasBaseColorMap 为 true 且纹理已正确绑定时才使用纹理
    // 注意：hasBaseColorMap 在 CPU 端已经确保只有在纹理 ID 有效时才为 true
    if (hasBaseColorMap) {
        vec4 texColor = texture(baseColorMap, TexCoords);
        // 直接使用纹理的 RGB 值，不考虑 alpha（alpha 用于透明度混合，不是颜色）
        // 如果纹理采样成功，使用纹理颜色；否则使用 baseColorFactor
        col = texColor.rgb;
        
        // 如果纹理颜色是纯白色 (1,1,1)，可能是纹理未正确绑定或采样失败
        // 在这种情况下，使用 baseColorFactor（如果 baseColorFactor 不是白色）
        // 但这里我们假设纹理绑定是正确的，所以直接使用纹理颜色
    }
    
    FragColor = vec4(col, 1.0);
}
