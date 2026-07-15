#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoord;

out vec4 FragColor;

// 材质贴图（glTF metallic-roughness）
uniform sampler2D uBaseColorMap;
uniform sampler2D uMetallicRoughnessMap;
uniform sampler2D uNormalMap;
uniform sampler2D uOcclusionMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uTransmissionMap;

uniform bool uHasBaseColorMap;
uniform bool uHasMetallicRoughnessMap;
uniform bool uHasNormalMap;
uniform bool uHasOcclusionMap;
uniform bool uHasEmissiveMap;
uniform bool uHasTransmission;
uniform bool uHasTransmissionMap;

uniform vec4 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3 uEmissiveFactor;
uniform float uNormalScale;
uniform float uOcclusionStrength;
uniform float uAlphaCutoff;
uniform int uAlphaMode;  // 0=OPAQUE, 1=MASK, 2=BLEND
uniform float uTransmissionFactor;

// 透射 / 折射（KHR_materials_transmission + ior + volume）
uniform float uIor;
uniform bool uHasIor;
uniform bool uHasVolume;
uniform float uVolumeThickness;
uniform vec3 uVolumeAttenuationColor;
uniform float uVolumeAttenuationDistance;

// 清漆（KHR_materials_clearcoat）
uniform bool uHasClearcoat;
uniform float uClearcoatFactor;
uniform float uClearcoatRoughness;
uniform float uClearcoatNormalScale;
uniform sampler2D uClearcoatMap;
uniform sampler2D uClearcoatRoughnessMap;
uniform sampler2D uClearcoatNormalMap;
uniform bool uHasClearcoatMap;
uniform bool uHasClearcoatRoughnessMap;
uniform bool uHasClearcoatNormalMap;

uniform vec3 uCameraPos;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform float uDirectLightScale;
uniform float uDiffuseEnvScale;
uniform float uSpecularEnvScale;
uniform float uHemiFillScale;

// IBL
uniform bool uUseIBL;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLUT;
uniform float uMaxReflectionLOD;

// 屏幕空间折射
uniform bool uOutputLinear;
uniform bool uHasSceneColor;
uniform sampler2D uSceneColorMap;
uniform vec2 uFramebufferSize;
uniform mat4 uView;
uniform mat4 uProjection;

const float PI = 3.14159265359;
const float EXPOSURE = 1.0;

// Narkowicz 2015 ACES 近似：压缩 HDR 高光，避免线性直出时过曝/压暗
vec3 toneMapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// 线性 → IEC 61966-2-1 sRGB（默认 LDR 帧缓冲假定按 sRGB 解释）
vec3 linearToSRGB(vec3 c) {
    bvec3 cutoff = lessThanEqual(c, vec3(0.0031308));
    vec3 higher = 1.055 * pow(max(c, vec3(0.0)), vec3(1.0 / 2.4)) - 0.055;
    vec3 lower = c * 12.92;
    return mix(higher, lower, vec3(cutoff));
}

vec3 toneMapAndEncode(vec3 linearColor) {
    return linearToSRGB(toneMapACES(linearColor * EXPOSURE));
}

vec3 getNormalFromMap() {
    if (!uHasNormalMap) {
        return normalize(vNormal);
    }
    vec3 tangentNormal = texture(uNormalMap, vTexCoord).xyz * 2.0 - 1.0;
    tangentNormal.xy *= uNormalScale;
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    return normalize(TBN * tangentNormal);
}

vec3 getClearcoatNormal() {
    if (!uHasClearcoatNormalMap) {
        return normalize(vNormal);
    }
    vec3 tangentNormal = texture(uClearcoatNormalMap, vTexCoord).xyz * 2.0 - 1.0;
    tangentNormal.xy *= uClearcoatNormalScale;
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 evaluateSpecularBRDF(vec3 N, vec3 V, vec3 L, vec3 H, vec3 F0, float roughness) {
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    return (NDF * G * F) / denom;
}

// AS4 Snell 折射：eta = index_i / index_t
bool snellRefract(vec3 incoming, vec3 normal, float eta, out vec3 transmitted) {
    vec3 n = normal;
    float cosi = dot(n, incoming);
    if (cosi > 0.0) {
        n = -n;
        cosi = -cosi;
    }
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    if (k < 0.0)
        return false;
    transmitted = normalize(eta * incoming - n * (eta * cosi + sqrt(k)));
    return true;
}

// glTF / 物理：正入射反射率 F0 = ((η-1)/(η+1))²；ior=1.5 → 0.04
float dielectricF0(float ior) {
    float f = (ior - 1.0) / (ior + 1.0);
    return f * f;
}

vec3 sampleSpecularIBL(vec3 N, vec3 V, float roughness, vec3 F0, float NdotV) {
    if (!uUseIBL)
        return vec3(0.0);
    vec3 R = reflect(-V, N);
    vec3 prefiltered = textureLod(uPrefilterMap, R, roughness * uMaxReflectionLOD).rgb;
    vec2 brdf = texture(uBrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    return prefiltered * (F * brdf.x + brdf.y);
}

vec3 sampleEnvironment(vec3 dir, float roughness) {
    if (!uUseIBL)
        return uAmbientColor;
    return textureLod(uPrefilterMap, dir, roughness * uMaxReflectionLOD).rgb;
}

// 屏幕空间折射：沿折射方向投影采样已绘制的不透明场景
vec3 sampleSceneRefraction(vec3 worldPos, vec3 refractDir, float roughness, float thickness) {
    float travel = max(thickness, 0.05);
    vec3 samplePos = worldPos + refractDir * travel;
    vec4 clip = uProjection * uView * vec4(samplePos, 1.0);
    if (abs(clip.w) < 1e-5)
        return sampleEnvironment(refractDir, roughness);

    vec2 ndc = clip.xy / clip.w;
    vec2 uv = ndc * 0.5 + 0.5;

    // 越界或背后：退回环境立方体贴图
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 || ndc.x * ndc.x + ndc.y * ndc.y > 2.5)
        return sampleEnvironment(refractDir, roughness);

    // 与当前片元屏幕位置混合少量偏移稳定性；粗糙度用 mip 模糊背景
    vec2 screenUV = gl_FragCoord.xy / max(uFramebufferSize, vec2(1.0));
    // IOR 偏离越大，折射采样点越远离几何投影
    uv = mix(screenUV, uv, clamp(travel * 2.0, 0.15, 1.0));

    float lod = roughness * 4.0;
    return textureLod(uSceneColorMap, uv, lod).rgb;
}

// AS4 风格体积吸收（Beer-Lambert）
vec3 applyVolumeAttenuation(vec3 radiance, float thickness) {
    if (!uHasVolume || uVolumeAttenuationDistance <= 0.0)
        return radiance;
    vec3 attColor = clamp(uVolumeAttenuationColor, vec3(1e-4), vec3(1.0));
    vec3 coeff = -log(attColor) / uVolumeAttenuationDistance;
    return radiance * exp(-coeff * max(thickness, 0.0));
}

void main() {
    vec4 baseColor = uBaseColorFactor;
    if (uHasBaseColorMap) {
        baseColor *= texture(uBaseColorMap, vTexCoord);
    }

    if (uAlphaMode == 1 && baseColor.a < uAlphaCutoff) {
        discard;
    }

    float metallic = uMetallicFactor;
    float roughness = uRoughnessFactor;
    if (uHasMetallicRoughnessMap) {
        vec3 mr = texture(uMetallicRoughnessMap, vTexCoord).rgb;
        metallic *= mr.b;
        roughness *= mr.g;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 N = getNormalFromMap();
    // doubleSided / 透明关闭剔除时，背面翻转法线（glTF 2.0）
    if (!gl_FrontFacing)
        N = -N;
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 L = normalize(-uLightDir);
    vec3 H = normalize(V + L);

    vec3 albedo = baseColor.rgb;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    vec3 specular = evaluateSpecularBRDF(N, V, L, H, F0, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 Lo = (kD * albedo / PI + specular) * uLightColor * NdotL * uDirectLightScale;

    vec3 diffuseIBL = vec3(0.0);
    vec3 specularIBL = vec3(0.0);
    if (uUseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (vec3(1.0) - F_ibl) * (1.0 - metallic);
        diffuseIBL = texture(uIrradianceMap, N).rgb * albedo * kD_ibl * uDiffuseEnvScale;
        specularIBL = sampleSpecularIBL(N, V, roughness, F0, NdotV) * uSpecularEnvScale;
        // 半球填充：模拟地面反弹，Sketchfab 工作室感（非 emissive）
        float hemi = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
        vec3 hemiFill = mix(vec3(0.20), vec3(0.36), hemi);
        diffuseIBL += hemiFill * albedo * kD_ibl * uHemiFillScale;
    } else {
        diffuseIBL = uAmbientColor * albedo * kD;
    }

    float ao = 1.0;
    if (uHasOcclusionMap) {
        ao = texture(uOcclusionMap, vTexCoord).r;
        ao = mix(1.0, ao, uOcclusionStrength);
    }

    // AO 只调制环境项，不混合直接光
    vec3 color = (diffuseIBL + specularIBL) * ao + Lo;

    if (uHasEmissiveMap) {
        color += texture(uEmissiveMap, vTexCoord).rgb * uEmissiveFactor;
    } else if (length(uEmissiveFactor) > 0.0) {
        color += uEmissiveFactor;
    }

    // KHR_materials_clearcoat：镜面层 + 底层能量扣除
    if (uHasClearcoat && uClearcoatFactor > 0.0) {
        float ccFactor = uClearcoatFactor;
        if (uHasClearcoatMap) {
            ccFactor *= texture(uClearcoatMap, vTexCoord).r;
        }

        float ccRough = uClearcoatRoughness;
        if (uHasClearcoatRoughnessMap) {
            ccRough *= texture(uClearcoatRoughnessMap, vTexCoord).g;
        }
        ccRough = clamp(ccRough, 0.04, 1.0);

        vec3 ccN = getClearcoatNormal();
        if (!gl_FrontFacing)
            ccN = -ccN;
        vec3 ccH = normalize(V + L);
        vec3 ccF0 = vec3(0.04);
        float ccNdotV = max(dot(ccN, V), 0.0);
        vec3 clearcoatFresnel = fresnelSchlick(ccNdotV, ccF0);

        // 底层被清漆 Fresnel 遮挡
        color *= (1.0 - ccFactor * clearcoatFresnel);

        vec3 ccSpec = evaluateSpecularBRDF(ccN, V, L, ccH, ccF0, ccRough);
        float ccNdotL = max(dot(ccN, L), 0.0);
        color += ccSpec * ccFactor * uLightColor * ccNdotL * uDirectLightScale;
        color += sampleSpecularIBL(ccN, V, ccRough, ccF0, ccNdotV) * ccFactor * uSpecularEnvScale;
    }

    float outAlpha = baseColor.a;

    // KHR_materials_transmission：环境反射 + 屏幕空间折射（Framebuffer）
    if (uHasTransmission && uTransmissionFactor > 0.0 && metallic < 0.99) {
        float transmission = uTransmissionFactor;
        if (uHasTransmissionMap) {
            transmission *= texture(uTransmissionMap, vTexCoord).r;
        }
        transmission = clamp(transmission, 0.0, 1.0);

        if (transmission > 0.001) {
            float ior = max(uIor, 1.0);

            // 薄壁双面：根据朝向选择 eta（空气 <-> 介质）
            vec3 refractN = N;
            float eta = 1.0 / ior;
            if (dot(N, V) < 0.0) {
                refractN = -N;
                eta = ior;
            }

            float F0d = dielectricF0(ior);
            float Fresnel = fresnelSchlick(NdotV, vec3(F0d)).r;

            // 透射替换漫反射：从表面颜色中去掉对应漫射能量
            color *= (1.0 - transmission);

            vec3 reflected = sampleEnvironment(reflect(-V, N), roughness);

            vec3 incoming = -V;
            vec3 refractDir;
            vec3 refracted;
            float thick = uHasVolume ? max(uVolumeThickness, 0.05) : mix(0.08, 0.25, clamp(ior - 1.0, 0.0, 1.0));

            if (snellRefract(incoming, refractN, eta, refractDir)) {
                if (uHasSceneColor) {
                    refracted = sampleSceneRefraction(vWorldPos, refractDir, roughness, thick);
                } else {
                    refracted = sampleEnvironment(refractDir, roughness);
                }
                refracted = applyVolumeAttenuation(refracted, thick);
                refracted *= albedo;
            } else {
                refracted = reflected;
            }

            // Fresnel 混合反射与折射
            vec3 glassColor = reflected * Fresnel + refracted * (1.0 - Fresnel);
            color += transmission * glassColor;

            // 透射已合成背景：用不透明 alpha，避免与 framebuffer 二次混合变暗
            outAlpha = mix(baseColor.a, 1.0, transmission);
        }
    }

    if (uOutputLinear) {
        FragColor = vec4(color, outAlpha);
    } else {
        FragColor = vec4(toneMapAndEncode(color), outAlpha);
    }
}
