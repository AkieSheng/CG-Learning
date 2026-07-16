#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vTexCoord;
in vec4 vLightSpacePos;

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

// 方向光阴影
uniform bool uUseShadows;
uniform sampler2D uShadowMap;
uniform float uShadowBias;

// 工作室水平灯带（沿长度采样近似）
uniform bool uUseLightStrips;
uniform int uLightStripCount;
uniform vec3 uStripCenter[3];
uniform vec3 uStripHalfRight[3];
uniform vec3 uStripHalfUp[3];
uniform vec3 uStripNormal[3];
uniform vec3 uStripColor[3];

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

// 方向光 shadow map：透视除法 + slope bias + 3x3 PCF
float sampleShadow(vec3 N, vec3 L) {
    if (!uUseShadows)
        return 1.0;

    vec3 projCoords = vLightSpacePos.xyz / max(vLightSpacePos.w, 1e-6);
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }

    float bias = max(uShadowBias * (1.0 - max(dot(N, L), 0.0)), uShadowBias * 0.25);
    float currentDepth = projCoords.z - bias;
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > closest ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
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

// 单条灯带：沿长度 5 点采样，做点光衰减的 Cook-Torrance
void accumulateStripLight(int stripIndex, vec3 N, vec3 V, vec3 albedo, vec3 F0,
                          float roughness, vec3 kD,
                          inout vec3 diffuseOut, inout vec3 specularOut) {
    vec3 center = uStripCenter[stripIndex];
    vec3 halfR = uStripHalfRight[stripIndex];
    vec3 halfU = uStripHalfUp[stripIndex];
    vec3 stripN = normalize(uStripNormal[stripIndex]);
    vec3 radiance = uStripColor[stripIndex];

    const int SAMPLES = 5;
    for (int i = 0; i < SAMPLES; ++i) {
        float t = (float(i) / float(SAMPLES - 1)) * 2.0 - 1.0;
        vec3 samplePos = center + halfR * t;
        vec3 toLight = samplePos - vWorldPos;
        float dist2 = max(dot(toLight, toLight), 1e-4);
        float dist = sqrt(dist2);
        vec3 L = toLight / dist;

        float facing = max(-dot(stripN, L), 0.0);
        if (facing <= 0.0)
            continue;

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        // 距离衰减 + 灯带面积权重
        float area = max(2.0 * length(halfR) * 2.0 * length(halfU), 1e-4);
        float atten = (area / float(SAMPLES)) * facing / (dist2 + 0.15);
        vec3 lightColor = radiance * atten;

        vec3 H = normalize(V + L);
        vec3 specular = evaluateSpecularBRDF(N, V, L, H, F0, roughness);
        diffuseOut += kD * albedo / PI * lightColor * NdotL;
        specularOut += specular * lightColor * NdotL;
    }
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

    vec3 diffuseDirect = kD * albedo / PI * uLightColor * NdotL * uDirectLightScale;
    vec3 specularDirect = specular * uLightColor * NdotL * uDirectLightScale;
    float shadow = sampleShadow(N, L);
    diffuseDirect *= shadow;
    specularDirect *= shadow;

    if (uUseLightStrips) {
        for (int i = 0; i < 3; ++i) {
            if (i >= uLightStripCount)
                break;
            accumulateStripLight(i, N, V, albedo, F0, roughness, kD,
                                 diffuseDirect, specularDirect);
        }
    }

    vec3 diffuseIBL = vec3(0.0);
    vec3 specularIBL = vec3(0.0);
    if (uUseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (vec3(1.0) - F_ibl) * (1.0 - metallic);
        diffuseIBL = texture(uIrradianceMap, N).rgb * albedo * kD_ibl * uDiffuseEnvScale;
        specularIBL = sampleSpecularIBL(N, V, roughness, F0, NdotV) * uSpecularEnvScale;
        // 金属：略抬镜面峰，但中间调向 albedo 色相收敛，避免灰环境冲成橙黄
        if (metallic > 0.01) {
            float albLum = max(dot(albedo, vec3(0.2126, 0.7152, 0.0722)), 1e-4);
            vec3 metalChroma = albedo / albLum;
            float specLum = max(dot(specularIBL, vec3(0.2126, 0.7152, 0.0722)), 1e-4);
            // 压暗金属中间调、拉开高光与体色对比
            float midCompress = mix(1.0, 0.68 / (0.68 + specLum), metallic);
            specularIBL *= midCompress;
            float compressedLum = max(dot(specularIBL, vec3(0.2126, 0.7152, 0.0722)), 1e-4);
            // 纠正色相，不再二次乘 albedo（F0 已含金属色）
            specularIBL = mix(specularIBL, metalChroma * compressedLum, metallic * 0.55);
            specularIBL *= (1.0 + 0.15 * metallic);
        }
        // 半球填充：模拟地面反弹
        float hemi = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
        vec3 hemiFill = mix(vec3(0.14), vec3(0.26), hemi);
        diffuseIBL += hemiFill * albedo * kD_ibl * uHemiFillScale;
    } else {
        diffuseIBL = uAmbientColor * albedo * kD;
    }

    float ao = 1.0;
    if (uHasOcclusionMap) {
        ao = texture(uOcclusionMap, vTexCoord).r;
        ao = mix(1.0, ao, uOcclusionStrength);
    }

    vec3 diffusePart = diffuseIBL * ao + diffuseDirect;
    vec3 specularPart = specularIBL * ao + specularDirect;
    vec3 color = diffusePart + specularPart;

    if (uHasEmissiveMap) {
        color += texture(uEmissiveMap, vTexCoord).rgb * uEmissiveFactor;
    } else if (length(uEmissiveFactor) > 0.0) {
        color += uEmissiveFactor;
    }

    float outAlpha = baseColor.a;

    // KHR_materials_transmission：仅替换漫反射能量，保留镜面高光
    if (uHasTransmission && uTransmissionFactor > 0.0 && metallic < 0.99) {
        float transmission = uTransmissionFactor;
        if (uHasTransmissionMap) {
            transmission *= texture(uTransmissionMap, vTexCoord).r;
        }
        transmission = clamp(transmission, 0.0, 1.0);

        if (transmission > 0.001) {
            float ior = max(uIor, 1.0);

            vec3 refractN = N;
            float eta = 1.0 / ior;
            if (dot(N, V) < 0.0) {
                refractN = -N;
                eta = ior;
            }

            float F0d = dielectricF0(ior);
            float Fresnel = fresnelSchlick(NdotV, vec3(F0d)).r;
            float tintLum = dot(albedo, vec3(0.2126, 0.7152, 0.0722));
            float tintStrength = clamp(1.0 - tintLum, 0.0, 1.0);
            vec3 lensTint = max(albedo, vec3(0.02));
            float frontWeight = pow(NdotV, 0.55);
            float grazingWeight = pow(1.0 - NdotV, 2.0);

            // 染色镜片掠射反射用更高 roughness，避免 prefilter 窄亮斑打成白条
            float reflectRough = max(roughness, mix(0.12, 0.42, grazingWeight * tintStrength));
            vec3 reflected = sampleEnvironment(reflect(-V, N), reflectRough);
            reflected *= mix(vec3(1.0), lensTint * 3.2, tintStrength);
            // 限制染色镜片反射亮度，根因：HDR 环境峰 + Fresnel 在侧面趋近 1
            float refLum = dot(reflected, vec3(0.2126, 0.7152, 0.0722));
            float tintRefLum = max(dot(lensTint, vec3(0.2126, 0.7152, 0.0722)), 0.02);
            float maxRefLum = tintRefLum * mix(3.0, 0.55, tintStrength);
            if (refLum > maxRefLum)
                reflected *= maxRefLum / refLum;

            vec3 incoming = -V;
            vec3 refractDir;
            vec3 refracted;
            float thick = uHasVolume ? max(uVolumeThickness, 0.05)
                                     : mix(0.28, 0.62, tintStrength);

            if (snellRefract(incoming, refractN, eta, refractDir)) {
                if (uHasSceneColor) {
                    refracted = sampleSceneRefraction(vWorldPos, refractDir, roughness, thick);
                } else {
                    refracted = sampleEnvironment(refractDir, roughness);
                }
                refracted = applyVolumeAttenuation(refracted, thick);
                refracted *= lensTint;
                if (!uHasVolume && tintStrength > 0.05) {
                    vec3 attColor = max(lensTint, vec3(0.03));
                    vec3 coeff = -log(attColor) / mix(0.08, 0.028, tintStrength);
                    refracted *= exp(-coeff * thick * mix(1.15, 2.6, frontWeight));
                }
            } else {
                refracted = reflected;
            }

            vec3 glassColor = reflected * Fresnel + refracted * (1.0 - Fresnel);
            // 正面压暗：透射路径仍偏亮时拉回更深的镜片底色
            glassColor = mix(glassColor, lensTint * mix(0.08, 0.28, frontWeight),
                             tintStrength * transmission * mix(0.70, 0.95, frontWeight));
            // 侧面压白：掠射时向更深 tint 收敛
            glassColor = mix(glassColor, lensTint * 0.32, grazingWeight * tintStrength * 0.90);

            diffusePart = mix(diffusePart, glassColor, transmission);

            // 透射已含 Fresnel 反射，削弱独立镜面以免侧面双重高光
            float specKeep = 1.0 - transmission * mix(0.50, 0.90, tintStrength);
            specularPart *= specKeep;
            color = diffusePart + specularPart;

            float lensAlpha = max(baseColor.a, 0.18 + 0.72 * tintStrength);
            outAlpha = mix(baseColor.a, lensAlpha, transmission);
        }
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
        color += ccSpec * ccFactor * uLightColor * ccNdotL * uDirectLightScale * shadow;
        color += sampleSpecularIBL(ccN, V, ccRough, ccF0, ccNdotV) * ccFactor * uSpecularEnvScale;
    }

    if (uOutputLinear) {
        FragColor = vec4(color, outAlpha);
    } else {
        FragColor = vec4(toneMapAndEncode(color), outAlpha);
    }
}
