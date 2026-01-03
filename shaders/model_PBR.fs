#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D baseColorMap;
uniform bool hasBaseColorMap;
uniform vec4 baseColorFactor;

uniform sampler2D normalMap;
uniform bool hasNormalMap;

uniform sampler2D metallicRoughnessMap; // glTF combined: G=roughness, B=metallic
uniform bool hasMetallicRoughnessMap;

uniform sampler2D metallicMap; // fallback separated
uniform bool hasMetallicMap;
uniform sampler2D roughnessMap; // fallback separated
uniform bool hasRoughnessMap;

uniform sampler2D aoMap;
uniform bool hasAOMap;

uniform sampler2D emissiveMap;
uniform bool hasEmissiveMap;
uniform vec3 emissiveFactor;

uniform float metallicFactor;
uniform float roughnessFactor;

uniform bool alphaModeBlend;
uniform bool alphaModeMask;
uniform float alphaCutoff;

uniform vec3 lightPos;
uniform vec3 viewPos;

const float PI = 3.14159265359;

vec3 getAlbedo()
{
    vec4 c = baseColorFactor;
    if (hasBaseColorMap) {
        c *= texture(baseColorMap, TexCoords);
    }
    // glTF baseColor is in sRGB; this is a minimal correction.
    return pow(c.rgb, vec3(2.2));
}

float getAlpha()
{
    float a = baseColorFactor.a;
    if (hasBaseColorMap) {
        a *= texture(baseColorMap, TexCoords).a;
    }
    return a;
}

vec3 getNormal()
{
    vec3 N = normalize(Normal);
    if (!hasNormalMap) return N;

    // Tangent-space normal map (assume OpenGL convention)
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    // If tangents are missing/degenerate, fall back to interpolated normal
    if (length(TBN[0]) < 1e-5 || length(TBN[1]) < 1e-5) return N;
    return normalize(TBN * tangentNormal);
}

float getMetallic()
{
    float m = metallicFactor;
    if (hasMetallicRoughnessMap) {
        m *= texture(metallicRoughnessMap, TexCoords).b;
    } else if (hasMetallicMap) {
        m *= texture(metallicMap, TexCoords).r;
    }
    return clamp(m, 0.0, 1.0);
}

float getRoughness()
{
    float r = roughnessFactor;
    if (hasMetallicRoughnessMap) {
        r *= texture(metallicRoughnessMap, TexCoords).g;
    } else if (hasRoughnessMap) {
        r *= texture(roughnessMap, TexCoords).r;
    }
    // Avoid 0 roughness (specular fireflies)
    return clamp(r, 0.04, 1.0);
}

float getAO()
{
    if (!hasAOMap) return 1.0;
    return texture(aoMap, TexCoords).r;
}

vec3 getEmissive()
{
    vec3 e = emissiveFactor;
    if (hasEmissiveMap) {
        e += texture(emissiveMap, TexCoords).rgb;
    }
    // emissive textures are usually sRGB too
    return pow(e, vec3(2.2));
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / max(denom, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    float alpha = getAlpha();
    if (alphaModeMask && alpha < alphaCutoff) discard;

    vec3  albedo    = getAlbedo();
    float metallic  = getMetallic();
    float roughness = getRoughness();
    float ao        = getAO();
    vec3  emissive  = getEmissive();

    vec3 N = getNormal();
    vec3 V = normalize(viewPos - FragPos);

    // Point light
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / max(distance * distance, 1e-3);
    vec3 radiance = vec3(1.0) * attenuation;

    // Cook-Torrance BRDF
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator   = NDF * G * F;
    float denom       = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3  specular    = numerator / denom;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // Minimal ambient (no IBL)
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo + emissive;

    // Tonemap + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    if (!alphaModeBlend) {
        alpha = 1.0;
    }
    FragColor = vec4(color, alpha);
}
