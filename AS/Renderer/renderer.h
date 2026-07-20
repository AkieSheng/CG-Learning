#pragma once

#include "scene.h"
#include "shader_program.h"
#include "ibl.h"
#include "matrix.h"

struct Renderer final {
  Renderer();
  ~Renderer();

  auto initialize(int width, int height) -> bool;
  auto resize(int width, int height) -> void;
  auto render(Scene& scene) -> void;
  auto destroy() -> void;

  auto setLightDirection(Vec3f const& dir) -> void;
  auto setLightColor(Vec3f const& color) -> void;
  auto setAmbientColor(Vec3f const& color) -> void;
  auto toggleFXAA() -> bool;
  auto cycleSupersampling() -> float;

  ShaderProgram pbrShader{};
  ShaderProgram skyboxShader{};
  ShaderProgram tonemapShader{};
  ShaderProgram shadowShader{};
  IBL ibl{};

  unsigned int msaaFBO{};
  unsigned int msaaColorRbo{};
  unsigned int msaaDepthRbo{};
  int msaaSamples{};

  unsigned int resolveFBO{};
  unsigned int sceneColorTex{};
  unsigned int sceneSampleTex{};
  unsigned int sceneDepthRbo{};

  unsigned int shadowFBO{};
  unsigned int shadowDepthTex{};
  unsigned int shadowDepthRbo{};
  int shadowMapSize{DEFAULT_SHADOW_MAP_SIZE};
  Matrix lightViewProjection{};
  bool shadowsEnabled{};

  unsigned int fullscreenVAO{};

  int viewportWidth{800};
  int viewportHeight{600};
  int renderWidth{1200};
  int renderHeight{900};
  int renderScaleMode{1};
  bool fxaaEnabled{};

  Vec3f lightDirection{0.54f, -0.50f, -0.68f};
  Vec3f lightColor{1.0f, 1.0f, 1.0f};
  Vec3f ambientColor{0.03f, 0.03f, 0.035f};

  static int const SCENE_SAMPLE_UNIT = 14;
  static int const SHADOW_MAP_UNIT = 15;
  static int const TARGET_MSAA_SAMPLES = 8;
  static int const DEFAULT_SHADOW_MAP_SIZE = 2048;

  auto loadShaders() -> bool;
  auto setupGLState() -> void;
  auto createSceneTargets(int width, int height) -> bool;
  auto destroySceneTargets() -> void;
  auto createShadowMap() -> bool;
  auto destroyShadowMap() -> void;
  auto updateRenderSize() -> void;
  auto currentRenderScale() const -> float;
  auto computeLightMatrix(Scene& scene) -> void;
  auto renderShadowMap(std::vector<Mesh*> const& opaque) -> void;
  auto bindCommonPBRUniforms(Scene& scene) -> void;
  auto drawMeshes(Scene& scene, bool transparentPassOnly,
                  std::vector<Mesh*> const* opaque,
                  std::vector<Mesh*> const* transparent) -> void;
  auto drawSingleMesh(Mesh* mesh, bool transparentPass) -> void;
  auto resolveMsaaToSceneColor() -> void;
  auto captureSceneColorSample() -> void;
  auto blitTonemapToScreen() -> void;

  static auto isTransparentMesh(Mesh const* mesh) -> bool;
  static auto meshSortKey(Mesh const* mesh, Vec3f const& camPos) -> float;
};
