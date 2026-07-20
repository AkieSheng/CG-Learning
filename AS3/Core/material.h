#pragma once

#include "vectors.h"
#include "ray.h"
#include "hit.h"

struct Material {
  virtual ~Material() {}

  virtual auto getDiffuseColor() const -> Vec3f = 0;
  virtual auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
                     Vec3f const& lightColor) const -> Vec3f = 0;
  virtual auto glSetMaterial() const -> void = 0;
};

struct PhongMaterial final : Material {
  PhongMaterial(Vec3f const& diffuseColor, Vec3f const& specularColor,
                float exponent);

  auto getDiffuseColor() const -> Vec3f override { return diffuseColor; }
  auto getSpecularColor() const -> Vec3f { return specularColor; }
  auto getExponent() const -> float { return exponent; }

  auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
             Vec3f const& lightColor) const -> Vec3f override;
  auto glSetMaterial() const -> void override;

  Vec3f diffuseColor{};
  Vec3f specularColor{};
  float exponent{};
};
