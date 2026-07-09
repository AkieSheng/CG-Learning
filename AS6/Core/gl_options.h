#ifndef _GL_OPTIONS_H_
#define _GL_OPTIONS_H_

// OpenGL 预览与光线追踪共用的命令行全局选项
extern int tessellation_theta;  // 球体细分：经度方向步数（theta ∈ [0, 360°]）
extern int tessellation_phi;    // 球体细分：纬度方向步数（phi ∈ [-90°, 90°]）
extern bool gouraud_shading;    // true：OpenGL 球体用顶点真法线；false：flat 面法线
extern bool specular_fix;       // true：光线追踪高光乘以 N·L，缓解掠射角 artifact

#endif
