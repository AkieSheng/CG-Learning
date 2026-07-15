#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "scene.h"
#include "shader_program.h"
#include "ibl.h"

// PBR 渲染器：MSAA HDR FBO + 屏幕空间折射 + ACES/sRGB + FXAA
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
  void updateRenderSize();
  float currentRenderScale() const;
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
  static const int TARGET_MSAA_SAMPLES = 8;
};

#endif
