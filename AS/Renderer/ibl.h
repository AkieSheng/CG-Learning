#pragma once

#include "shader_program.h"

struct IBL final {
  static int const IRRADIANCE_UNIT = 8;
  static int const PREFILTER_UNIT = 9;
  static int const BRDF_LUT_UNIT = 10;

  IBL();
  ~IBL();

  auto initialize() -> bool;
  auto destroy() -> void;

  auto renderSkybox(ShaderProgram const& skyboxShader, float const* view,
                    float const* projection) const -> void;
  auto bindForPBR(ShaderProgram& pbrShader) const -> void;

  auto valid() const -> bool { return envCubemap != 0; }
  auto maxReflectionLod() const -> float { return maxReflectionLod_; }

  unsigned int envCubemap{};
  unsigned int skyboxCubemap{};
  unsigned int irradianceMap{};
  unsigned int prefilterMap{};
  unsigned int brdfLUT{};
  unsigned int skyboxVAO{};
  unsigned int skyboxVBO{};
  unsigned int captureFBO{};

  ShaderProgram brdfLUTShader{};
  float maxReflectionLod_{};

  IBL(IBL const&) = delete;
  auto operator=(IBL const&) -> IBL& = delete;

  auto createEnvironmentCubemap() -> bool;
  auto createSkyboxCubemap() -> bool;
  auto createIrradianceMap() -> bool;
  auto createPrefilterMap() -> bool;
  auto createBrdfLUT() -> bool;
  auto createSkyboxVAO() -> bool;
};
