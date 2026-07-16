#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "scene.h"
#include "shader_program.h"
#include "ibl.h"
#include "matrix.h"

// PBR 渲染器：MSAA HDR FBO + 方向光阴影 + 屏幕空间折射 + ACES/sRGB + FXAA
class Renderer {
public:
  Renderer();
  ~Renderer();

  bool initialize(int width, int height);
  void resize(int width, int height);
  void render(Scene &scene);
  void destroy();

  void setLightDirection(const Vec3f &dir);
  void setLightColor(const Vec3f &color);
  void setAmbientColor(const Vec3f &color);
  bool toggleFXAA();
  float cycleSupersampling();

private:
  bool loadShaders();
  void setupGLState();
  bool createSceneTargets(int width, int height);
  void destroySceneTargets();
  bool createShadowMap();
  void destroyShadowMap();
  void updateRenderSize();
  float currentRenderScale() const;
  void computeLightMatrix(Scene &scene);
  void renderShadowMap(const std::vector<Mesh *> &opaque);
  void bindCommonPBRUniforms(Scene &scene);
  void drawMeshes(Scene &scene, bool transparentPassOnly,
                  const std::vector<Mesh *> *opaque,
                  const std::vector<Mesh *> *transparent);
  void drawSingleMesh(Mesh *mesh, bool transparentPass);
  void resolveMsaaToSceneColor();
  void captureSceneColorSample();
  void blitTonemapToScreen();

  static bool isTransparentMesh(const Mesh *mesh);
  static float meshSortKey(const Mesh *mesh, const Vec3f &camPos);

  ShaderProgram pbrShader;
  ShaderProgram skyboxShader;
  ShaderProgram tonemapShader;
  ShaderProgram shadowShader;
  IBL ibl;

  // MSAA HDR 场景缓冲
  unsigned int msaaFBO;
  unsigned int msaaColorRbo;
  unsigned int msaaDepthRbo;
  int msaaSamples;

  // 单采样 resolve 目标（tonemap / 折射源）
  unsigned int resolveFBO;
  unsigned int sceneColorTex;
  unsigned int sceneSampleTex;
  // 无 MSAA 时的单采样深度
  unsigned int sceneDepthRbo;

  // 方向光 shadow map（颜色纹理存深度 + depth RBO 测试）
  unsigned int shadowFBO;
  unsigned int shadowDepthTex;
  unsigned int shadowDepthRbo;
  int shadowMapSize;
  Matrix lightViewProjection;
  bool shadowsEnabled;

  unsigned int fullscreenVAO;

  int viewportWidth;
  int viewportHeight;
  int renderWidth;
  int renderHeight;
  int renderScaleMode;
  bool fxaaEnabled;

  Vec3f lightDirection;
  Vec3f lightColor;
  Vec3f ambientColor;

  static const int SCENE_SAMPLE_UNIT = 14;
  static const int SHADOW_MAP_UNIT = 15;
  static const int TARGET_MSAA_SAMPLES = 8;
  static const int DEFAULT_SHADOW_MAP_SIZE = 2048;
};

#endif
