#pragma once

#include "ray.h"
#include "vectors.h"

struct Camera
{
  virtual ~Camera() {}
  virtual auto generateRay(Vec2f point) -> Ray = 0;
  virtual auto getTMin() const -> float = 0;
};

struct OrthographicCamera final : Camera
{
  OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size);
  auto generateRay(Vec2f point) -> Ray override;
  auto getTMin() const -> float override;

  Vec3f center{};
  Vec3f direction{};
  Vec3f up{};
  Vec3f horizontal{};
  float size{};
};
