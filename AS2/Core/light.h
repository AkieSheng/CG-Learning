#pragma once

#include "vectors.h"
#include "object3d.h"

struct Light {
  Light() {}
  virtual ~Light() {}

  virtual auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col) const -> void = 0;
};

struct DirectionalLight final : Light {
  DirectionalLight(Vec3f const& d, Vec3f const& c) {
    direction = d;
    direction.Normalize();
    color = c;
  }
  ~DirectionalLight() {}

  auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col) const -> void override {
    dir = direction * (-1.0f);
    col = color;
  }

  DirectionalLight() = delete;

  Vec3f direction{};
  Vec3f color{};
};
