#pragma once

#include "material.h"

struct Matrix;

struct Noise final : Material {
  Noise(Matrix* m, Material* mat1, Material* mat2, int octaves);
  ~Noise() override;

  auto getDiffuseColor(Vec3f const& point) const -> Vec3f override;
  auto getSpecularColor(Vec3f const& point) const -> Vec3f override;
  auto getExponent(Vec3f const& point) const -> float override;
  auto getReflectiveColor(Vec3f const& point) const -> Vec3f override;
  auto getTransparentColor(Vec3f const& point) const -> Vec3f override;
  auto getIndexOfRefraction(Vec3f const& point) const -> float override;

  auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
             Vec3f const& lightColor) const -> Vec3f override;
  auto glSetMaterial() const -> void override;

  auto blendWeight(Vec3f const& worldPoint) const -> float;

  Matrix* mapping{};
  Material* mat1{};
  Material* mat2{};
  int octaves{};
};
