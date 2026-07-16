#version 330 core

// 将深度写入颜色附件
out vec4 FragColor;

void main() {
    FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
