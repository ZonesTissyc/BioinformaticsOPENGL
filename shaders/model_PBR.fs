#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D baseColorMap;
uniform bool hasBaseColorMap;
uniform vec3 baseColorFactor;

void main() {
    vec3 col = baseColorFactor;
    if (hasBaseColorMap) {
        col = texture(baseColorMap, TexCoords).rgb;
    }
    FragColor = vec4(col, 1.0);
}
