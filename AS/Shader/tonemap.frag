#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uHdrColor;
uniform vec2 uFramebufferSize;
uniform bool uEnableFXAA;

const float EXPOSURE = 0.92;

// 保守 FXAA 参数：优先去边缘阶梯，避免过量糊化
const float FXAA_SPAN_MAX = 8.0;
const float FXAA_REDUCE_MUL = 1.0 / 8.0;
const float FXAA_REDUCE_MIN = 1.0 / 128.0;

vec3 toneMapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 linearToSRGB(vec3 c) {
    bvec3 cutoff = lessThanEqual(c, vec3(0.0031308));
    vec3 higher = 1.055 * pow(max(c, vec3(0.0)), vec3(1.0 / 2.4)) - 0.055;
    vec3 lower = c * 12.92;
    return mix(higher, lower, vec3(cutoff));
}

vec3 sampleTonemapped(vec2 uv) {
    vec3 hdr = texture(uHdrColor, uv).rgb;
    return linearToSRGB(toneMapACES(hdr * EXPOSURE));
}

float luma(vec3 c) {
    return dot(c, vec3(0.299, 0.587, 0.114));
}

// NVIDIA FXAA 简化版：在 LDR sRGB 上做边缘检测与亚像素混合
vec3 applyFXAA(vec2 uv) {
    vec2 texel = 1.0 / max(uFramebufferSize, vec2(1.0));

    vec3 rgbNW = sampleTonemapped(uv + vec2(-1.0, -1.0) * texel);
    vec3 rgbNE = sampleTonemapped(uv + vec2( 1.0, -1.0) * texel);
    vec3 rgbSW = sampleTonemapped(uv + vec2(-1.0,  1.0) * texel);
    vec3 rgbSE = sampleTonemapped(uv + vec2( 1.0,  1.0) * texel);
    vec3 rgbM  = sampleTonemapped(uv);

    float lumaNW = luma(rgbNW);
    float lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW);
    float lumaSE = luma(rgbSE);
    float lumaM  = luma(rgbM);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
                          FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, vec2(-FXAA_SPAN_MAX), vec2(FXAA_SPAN_MAX)) * texel;

    vec3 rgbA = 0.5 * (
        sampleTonemapped(uv + dir * (1.0 / 3.0 - 0.5)) +
        sampleTonemapped(uv + dir * (2.0 / 3.0 - 0.5)));
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        sampleTonemapped(uv + dir * -0.5) +
        sampleTonemapped(uv + dir *  0.5));

    float lumaB = luma(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax)
        return rgbA;
    return rgbB;
}

void main() {
    if (uEnableFXAA) {
        FragColor = vec4(applyFXAA(vTexCoord), 1.0);
    } else {
        FragColor = vec4(sampleTonemapped(vTexCoord), 1.0);
    }
}
