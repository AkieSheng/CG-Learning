#pragma once

#include "texture.h"
#include "shader_program.h"
#include <string>

enum AlphaMode
{
  ALPHA_OPAQUE = 0,
  ALPHA_MASK,
  ALPHA_BLEND
};

struct PBRMaterial final
{
  std::string name{};

  float baseColorFactor[4]{1.0f, 1.0f, 1.0f, 1.0f};
  float metallicFactor{1.0f};
  float roughnessFactor{1.0f};
  float emissiveFactor[3]{};
  float normalScale{1.0f};
  float occlusionStrength{1.0f};
  float alphaCutoff{0.5f};

  Texture* baseColorTexture{};
  Texture* metallicRoughnessTexture{};
  Texture* normalTexture{};
  Texture* occlusionTexture{};
  Texture* emissiveTexture{};

  bool hasTransmission{};
  float transmissionFactor{};
  Texture* transmissionTexture{};
  float ior{1.5f};
  bool hasIor{};

  bool hasVolume{};
  float volumeThickness{};
  float volumeAttenuationColor[3]{1.0f, 1.0f, 1.0f};
  float volumeAttenuationDistance{};

  bool hasClearcoat{};
  float clearcoatFactor{};
  float clearcoatRoughnessFactor{};
  Texture* clearcoatTexture{};
  Texture* clearcoatRoughnessTexture{};
  Texture* clearcoatNormalTexture{};
  float clearcoatNormalScale{1.0f};

  bool doubleSided{};
  AlphaMode alphaMode{ALPHA_OPAQUE};

  PBRMaterial();
  ~PBRMaterial();

  auto bindTextures(ShaderProgram& shader) const -> void;
  auto setUniforms(ShaderProgram& shader) const -> void;
};
