#pragma once

#include "material.h"

struct Matrix;
struct Wood;

struct Checkerboard final : Material {
  Checkerboard(Matrix* m, Material* mat1, Material* mat2);
  ~Checkerboard() override;

  auto getDiffuseColor(Vec3f const& point) const -> Vec3f override;
  auto getSpecularColor(Vec3f const& point) const -> Vec3f override;
  auto getExponent(Vec3f const& point) const -> float override;
  auto getReflectiveColor(Vec3f const& point) const -> Vec3f override;
  auto getTransparentColor(Vec3f const& point) const -> Vec3f override;
  auto getIndexOfRefraction(Vec3f const& point) const -> float override;

  auto Shade(Ray const& ray, Hit const& hit, Vec3f const& dirToLight,
             Vec3f const& lightColor) const -> Vec3f override;
  auto glSetMaterial() const -> void override;

private:
  auto selectMaterial(Vec3f const& worldPoint) const -> Material*;
  auto selectedWood(Vec3f const& worldPoint) const -> Wood const*;
  auto woodCellWeight(Vec3f const& worldPoint, Wood const* wood) const -> float;

  static float const FLOOR_WOOD_FREQ_BOOST;

  Matrix* mapping{};
  Material* mat1{};
  Material* mat2{};
};
