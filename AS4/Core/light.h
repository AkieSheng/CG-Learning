#pragma once

#include "vectors.h"
#include "object3d.h"

struct Light {
  Light() {}
  virtual ~Light() {}

  virtual auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col,
                               float& distanceToLight) const -> void = 0;
  virtual auto glInit(int id) -> void = 0;
};

struct DirectionalLight final : Light {
  DirectionalLight(Vec3f const& d, Vec3f const& c) {
    direction = d;
    direction.Normalize();
    color = c;
  }
  ~DirectionalLight() {}

  auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col,
                       float& distanceToLight) const -> void override {
    dir = direction * (-1.0f);
    col = color;
    distanceToLight = INFINITY;
  }

  auto glInit(int id) -> void override;

  DirectionalLight() = delete;

  Vec3f direction{};
  Vec3f color{};
};

struct PointLight final : Light {
  PointLight(Vec3f const& p, Vec3f const& c, float a1, float a2, float a3) {
    position = p;
    color = c;
    attenuation_1 = a1;
    attenuation_2 = a2;
    attenuation_3 = a3;
  }
  ~PointLight() {}

  auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col,
                       float& distanceToLight) const -> void override {
    dir = position - p;
    distanceToLight = dir.Length();
    dir.Normalize();
    auto attenuation = 1.0f / (attenuation_1 +
                               attenuation_2 * distanceToLight +
                               attenuation_3 * distanceToLight * distanceToLight);
    if (attenuation < 0.0f) {
      attenuation = 0.0f;
    }
    col = color * attenuation;
  }

  auto glInit(int id) -> void override;

  PointLight() = delete;

  Vec3f position{};
  Vec3f color{};
  float attenuation_1{};
  float attenuation_2{};
  float attenuation_3{};
};
