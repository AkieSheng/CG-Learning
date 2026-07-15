#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "scene.h"
#include "shader_program.h"
#include "ibl.h"

// PBR 渲染器：线性 HDR 场景 FBO + 屏幕空间折射 + 最终 ACES/sRGB
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

private:
  bool loadShaders();
  void setupGLState();
  bool createSceneTargets(int width, int height);
  void destroySceneTargets();
  void bindCommonPBRUniforms(Scene &scene);
  void drawMeshes(Scene &scene, bool transparentPassOnly,
                  const std::vector<Mesh *> *opaque,
                  const std::vector<Mesh *> *transparent);
  void drawSingleMesh(Mesh *mesh, bool transparentPass);
  void captureSceneColorSample();
  void blitTonemapToScreen();

  static bool isTransparentMesh(const Mesh *mesh);
  static float meshSortKey(const Mesh *mesh, const Vec3f &camPos);

  ShaderProgram pbrShader;
  ShaderProgram skyboxShader;
  ShaderProgram tonemapShader;
  IBL ibl;

  // 线性 HDR 场景
  unsigned int sceneFBO;
  unsigned int sceneColorTex;
  unsigned int sceneDepthRbo;
  // 不透明后的场景颜色拷贝，供屏幕空间折射采样
  unsigned int sceneSampleTex;
  unsigned int fullscreenVAO;

  // 视口宽高
  int viewportWidth;
  int viewportHeight;

  Vec3f lightDirection;
  Vec3f lightColor;
  Vec3f ambientColor;  // 环境光颜色

  static const int SCENE_SAMPLE_UNIT = 14;
};

#endif
