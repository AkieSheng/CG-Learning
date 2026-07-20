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
                float exponent, Vec3f const& reflectiveColor,
                Vec3f const& transparentColor, float indexOfRefraction);

  auto getDiffuseColor() const -> Vec3f override { return diffuseColor; }
  auto getSpecularColor() const -> Vec3f { return specularColor; }
  auto getExponent() const -> float { return exponent; }
  auto getReflectiveColor() const -> Vec3f { return reflectiveColor; }
  auto getTransparentColor() const -> Vec3f { return transparentColor; }
  auto getIndexOfRefraction() const -> float { return indexOfRefraction; }

  auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
             Vec3f const& lightColor) const -> Vec3f override;
  auto glSetMaterial() const -> void override;

  Vec3f diffuseColor{};
  Vec3f specularColor{};
  float exponent{};
  Vec3f reflectiveColor{};
  Vec3f transparentColor{};
  float indexOfRefraction{};
};
