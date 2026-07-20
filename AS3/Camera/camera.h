#pragma once

#include "ray.h"
#include "vectors.h"

struct Camera {
  virtual ~Camera() {}
  virtual auto generateRay(Vec2f point) -> Ray = 0;
  virtual auto getTMin() const -> float = 0;

  virtual auto glInit(int w, int h) -> void = 0;
  virtual auto glPlaceCamera() -> void = 0;
  virtual auto dollyCamera(float dist) -> void = 0;
  virtual auto truckCamera(float dx, float dy) -> void = 0;
  virtual auto rotateCamera(float rx, float ry) -> void = 0;
};

struct OrthographicCamera final : Camera {
  OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size);
  auto generateRay(Vec2f point) -> Ray override;
  auto getTMin() const -> float override;

  auto glInit(int w, int h) -> void override;
  auto glPlaceCamera() -> void override;
  auto dollyCamera(float dist) -> void override;
  auto truckCamera(float dx, float dy) -> void override;
  auto rotateCamera(float rx, float ry) -> void override;

  auto updateHorizontal() -> void;

  Vec3f center{};
  Vec3f direction{};
  Vec3f up{};
  Vec3f horizontal{};
  float size{};
};

struct PerspectiveCamera final : Camera {
  PerspectiveCamera(Vec3f center, Vec3f direction, Vec3f up, float angle);
  auto generateRay(Vec2f point) -> Ray override;
  auto getTMin() const -> float override;

  auto glInit(int w, int h) -> void override;
  auto glPlaceCamera() -> void override;
  auto dollyCamera(float dist) -> void override;
  auto truckCamera(float dx, float dy) -> void override;
  auto rotateCamera(float rx, float ry) -> void override;

  auto updateHorizontal() -> void;

  Vec3f center{};
  Vec3f direction{};
  Vec3f up{};
  Vec3f horizontal{};
  float angle{};
  float halfHeight{};
};
