#ifndef _IBL_H_
#define _IBL_H_

#include "shader_program.h"

// Image-Based Lighting
// 程序化环境 cubemap + 辐照度卷积 + GGX 镜面预滤波 + BRDF LUT
class IBL {
public:
  static const int IRRADIANCE_UNIT = 8;
  static const int PREFILTER_UNIT  = 9;
  static const int BRDF_LUT_UNIT   = 10;

  IBL();
  ~IBL();

  bool initialize();
  void destroy();

  void renderSkybox(const ShaderProgram &skyboxShader,
                    const float *view, const float *projection) const;
  void bindForPBR(ShaderProgram &pbrShader) const;

  bool valid() const { return envCubemap != 0; }
  float maxReflectionLod() const { return maxReflectionLod_; }

private:
  bool createEnvironmentCubemap();
  bool createSkyboxCubemap();
  bool createIrradianceMap();
  bool createPrefilterMap();
  bool createBrdfLUT();
  bool createSkyboxVAO();

  unsigned int envCubemap;  // 照明 HDR cubemap
  unsigned int skyboxCubemap;  // 天空盒显示 cubemap
  unsigned int irradianceMap;  // 漫反射辐照度 cubemap
  unsigned int prefilterMap;  // GGX 镜面预滤波 cubemap
  unsigned int brdfLUT;  // Split-Sum BRDF LUT
  unsigned int skyboxVAO;
  unsigned int skyboxVBO;
  unsigned int captureFBO;

  ShaderProgram brdfLUTShader;  // BRDF LUT 着色器
  float maxReflectionLod_;  // 最大反射 LOD 

  IBL(const IBL &);
  IBL &operator=(const IBL &);
};

#endif
