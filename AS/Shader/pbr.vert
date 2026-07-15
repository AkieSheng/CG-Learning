#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec2 aTexCoord;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec2 vTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    vNormal = normalize(uNormalMatrix * aNormal);
    vTangent = normalize(uNormalMatrix * aTangent.xyz);
    vBitangent = cross(vNormal, vTangent) * aTangent.w;
    vTexCoord = aTexCoord;

    gl_Position = uProjection * uView * worldPos;
}
