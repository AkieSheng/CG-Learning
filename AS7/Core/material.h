#pragma once

#include "vectors.h"
#include "ray.h"
#include "hit.h"

struct Material {
  virtual ~Material() {}

  virtual auto getDiffuseColor(Vec3f const& point) const -> Vec3f = 0;
  virtual auto getSpecularColor(Vec3f const& point) const -> Vec3f;
  virtual auto getExponent(Vec3f const& point) const -> float;
  virtual auto getReflectiveColor(Vec3f const& point) const -> Vec3f;
  virtual auto getTransparentColor(Vec3f const& point) const -> Vec3f;
  virtual auto getIndexOfRefraction(Vec3f const& point) const -> float;

  virtual auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
                     Vec3f const& lightColor) const -> Vec3f = 0;
};

struct PhongMaterial final : Material {
  PhongMaterial(Vec3f const& diffuseColor, Vec3f const& specularColor,
                float exponent, Vec3f const& reflectiveColor,
                Vec3f const& transparentColor, float indexOfRefraction);

  auto getDiffuseColor(Vec3f const& point) const -> Vec3f override;
  auto getSpecularColor(Vec3f const& point) const -> Vec3f override;
  auto getExponent(Vec3f const& point) const -> float override;
  auto getReflectiveColor(Vec3f const& point) const -> Vec3f override;
  auto getTransparentColor(Vec3f const& point) const -> Vec3f override;
  auto getIndexOfRefraction(Vec3f const& point) const -> float override;

  auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
             Vec3f const& lightColor) const -> Vec3f override;

  Vec3f diffuseColor{};
  Vec3f specularColor{};
  float exponent{};
  Vec3f reflectiveColor{};
  Vec3f transparentColor{};
  float indexOfRefraction{};
};
