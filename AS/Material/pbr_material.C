#include "pbr_material.h"
#include "shader_program.h"

PBRMaterial::PBRMaterial() {}

PBRMaterial::~PBRMaterial() {
  delete baseColorTexture;
  delete metallicRoughnessTexture;
  delete normalTexture;
  delete occlusionTexture;
  delete emissiveTexture;
  delete transmissionTexture;
  delete clearcoatTexture;
  delete clearcoatRoughnessTexture;
  delete clearcoatNormalTexture;
}

auto PBRMaterial::bindTextures(ShaderProgram& shader) const -> void {
  int unit = 0;

  if (baseColorTexture && baseColorTexture->valid()) {
    baseColorTexture->bind(unit);
    shader.setInt("uBaseColorMap", unit);
    shader.setBool("uHasBaseColorMap", true);
    unit++;
  } else {
    shader.setBool("uHasBaseColorMap", false);
  }

  if (metallicRoughnessTexture && metallicRoughnessTexture->valid()) {
    metallicRoughnessTexture->bind(unit);
    shader.setInt("uMetallicRoughnessMap", unit);
    shader.setBool("uHasMetallicRoughnessMap", true);
    unit++;
  } else {
    shader.setBool("uHasMetallicRoughnessMap", false);
  }

  if (normalTexture && normalTexture->valid()) {
    normalTexture->bind(unit);
    shader.setInt("uNormalMap", unit);
    shader.setBool("uHasNormalMap", true);
    unit++;
  } else {
    shader.setBool("uHasNormalMap", false);
  }

  if (occlusionTexture && occlusionTexture->valid()) {
    occlusionTexture->bind(unit);
    shader.setInt("uOcclusionMap", unit);
    shader.setBool("uHasOcclusionMap", true);
    unit++;
  } else {
    shader.setBool("uHasOcclusionMap", false);
  }

  if (emissiveTexture && emissiveTexture->valid()) {
    emissiveTexture->bind(unit);
    shader.setInt("uEmissiveMap", unit);
    shader.setBool("uHasEmissiveMap", true);
    unit++;
  } else {
    shader.setBool("uHasEmissiveMap", false);
  }

  if (hasTransmission && transmissionTexture && transmissionTexture->valid()) {
    transmissionTexture->bind(unit);
    shader.setInt("uTransmissionMap", unit);
    shader.setBool("uHasTransmissionMap", true);
    unit++;
  } else {
    shader.setBool("uHasTransmissionMap", false);
  }

  int const clearcoatUnitBase = 11;
  if (hasClearcoat && clearcoatTexture && clearcoatTexture->valid()) {
    clearcoatTexture->bind(clearcoatUnitBase);
    shader.setInt("uClearcoatMap", clearcoatUnitBase);
    shader.setBool("uHasClearcoatMap", true);
  } else {
    shader.setBool("uHasClearcoatMap", false);
  }
  if (hasClearcoat && clearcoatRoughnessTexture &&
      clearcoatRoughnessTexture->valid()) {
    clearcoatRoughnessTexture->bind(clearcoatUnitBase + 1);
    shader.setInt("uClearcoatRoughnessMap", clearcoatUnitBase + 1);
    shader.setBool("uHasClearcoatRoughnessMap", true);
  } else {
    shader.setBool("uHasClearcoatRoughnessMap", false);
  }
  if (hasClearcoat && clearcoatNormalTexture &&
      clearcoatNormalTexture->valid()) {
    clearcoatNormalTexture->bind(clearcoatUnitBase + 2);
    shader.setInt("uClearcoatNormalMap", clearcoatUnitBase + 2);
    shader.setBool("uHasClearcoatNormalMap", true);
  } else {
    shader.setBool("uHasClearcoatNormalMap", false);
  }
}

auto PBRMaterial::setUniforms(ShaderProgram& shader) const -> void {
  shader.setVec4("uBaseColorFactor", baseColorFactor[0], baseColorFactor[1],
                 baseColorFactor[2], baseColorFactor[3]);
  shader.setFloat("uMetallicFactor", metallicFactor);
  shader.setFloat("uRoughnessFactor", roughnessFactor);
  shader.setVec3("uEmissiveFactor", emissiveFactor[0], emissiveFactor[1],
                 emissiveFactor[2]);
  shader.setFloat("uNormalScale", normalScale);
  shader.setFloat("uOcclusionStrength", occlusionStrength);
  shader.setFloat("uAlphaCutoff", alphaCutoff);
  shader.setInt("uAlphaMode", static_cast<int>(alphaMode));
  shader.setBool("uHasTransmission", hasTransmission);
  shader.setFloat("uTransmissionFactor", transmissionFactor);
  shader.setFloat("uIor", ior);
  shader.setBool("uHasIor", hasIor);
  shader.setBool("uHasVolume", hasVolume);
  shader.setFloat("uVolumeThickness", volumeThickness);
  shader.setVec3("uVolumeAttenuationColor", volumeAttenuationColor[0],
                 volumeAttenuationColor[1], volumeAttenuationColor[2]);
  shader.setFloat("uVolumeAttenuationDistance", volumeAttenuationDistance);
  shader.setBool("uHasClearcoat", hasClearcoat);
  shader.setFloat("uClearcoatFactor", clearcoatFactor);
  shader.setFloat("uClearcoatRoughness", clearcoatRoughnessFactor);
  shader.setFloat("uClearcoatNormalScale", clearcoatNormalScale);
}
