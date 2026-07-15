#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uHdrColor;

const float EXPOSURE = 1.0;

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

void main() {
    vec3 hdr = texture(uHdrColor, vTexCoord).rgb;
    FragColor = vec4(linearToSRGB(toneMapACES(hdr * EXPOSURE)), 1.0);
}
