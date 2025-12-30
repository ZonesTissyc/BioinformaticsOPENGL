#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{   
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 透明度测试 - 完全透明的像素直接丢弃
    if(texColor.a < 0.01) discard;
    
    // 输出带透明度的颜色
    FragColor = texColor;
}
