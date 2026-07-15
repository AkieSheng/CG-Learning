#ifndef _PBR_MATERIAL_H_
#define _PBR_MATERIAL_H_

#include <string>
#include "texture.h"
#include "shader_program.h"

// 透明度模式
enum AlphaMode {
  ALPHA_OPAQUE = 0,  // 不透明
  ALPHA_MASK,  // 按 Alpha 阈值裁剪
  ALPHA_BLEND  // Alpha 混合
};

// PBR 材质（对应 glTF metallic-roughness 材质）
struct PBRMaterial {
  std::string name;

  // 因子
  float baseColorFactor[4];  // 无贴图时的 RGBA
  float metallicFactor;  // 金属度
  float roughnessFactor;  // 粗糙度
  float emissiveFactor[3];  // 自发光
  float normalScale;  // 法线强度
  float occlusionStrength;  // AO 强度
  float alphaCutoff;  // Alpha 阈值

  // 标准贴图（glTF 各通道贴图指针）
  Texture *baseColorTexture;
  Texture *metallicRoughnessTexture;
  Texture *normalTexture;
  Texture *occlusionTexture;
  Texture *emissiveTexture;

  // 透射拓展（玻璃）
  bool hasTransmission;
  float transmissionFactor;
  Texture *transmissionTexture;
  float ior;  // 折射率（KHR_materials_ior，默认 1.5）
  bool hasIor;

  // 体积吸收（KHR_materials_volume）
  bool hasVolume;
  float volumeThickness;
  float volumeAttenuationColor[3];
  float volumeAttenuationDistance;

  // 清漆拓展
  bool hasClearcoat;
  float clearcoatFactor;
  float clearcoatRoughnessFactor;
  Texture *clearcoatTexture;
  Texture *clearcoatRoughnessTexture;
  Texture *clearcoatNormalTexture;
  float clearcoatNormalScale;

  bool doubleSided;  // 是否双面渲染
  AlphaMode alphaMode;  // 透明度模式

  PBRMaterial();
  ~PBRMaterial();

  void bindTextures(class ShaderProgram &shader) const;
  void setUniforms(class ShaderProgram &shader) const;
};

#endif
