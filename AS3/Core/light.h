#pragma once

#include "vectors.h"
#include "object3d.h"


struct Light
{
  Light() {}
  virtual ~Light() {}

  virtual auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col,
                               float& distanceToLight) const -> void = 0;
  virtual auto glInit(int id) -> void = 0;
};

struct DirectionalLight final : Light
{
  DirectionalLight()
  {
    direction = Vec3f(0, 0, 0);
    color = Vec3f(1, 1, 1);
  }
  DirectionalLight(Vec3f const& d, Vec3f const& c)
  {
    direction = d;
    direction.Normalize();
    color = c;
  }
  ~DirectionalLight() {}

  auto getIllumination(Vec3f const& p, Vec3f& dir, Vec3f& col,
                       float& distanceToLight) const -> void override
  {
    dir = direction * (-1.0f);
    col = color;
    distanceToLight = INFINITY;
  }

  auto glInit(int id) -> void override;

  Vec3f direction{};
  Vec3f color{};
};
