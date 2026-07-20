#include "camera.h"
#include <cmath>

static auto buildCameraBasis(Vec3f& direction, Vec3f& up, Vec3f& horizontal) -> void {
  direction.Normalize();

  up -= direction * up.Dot3(direction);
  up.Normalize();

  Vec3f::Cross3(horizontal, direction, up);
  horizontal.Normalize();
}

OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s) {
  center = c;
  size = s;
  direction = dir;
  up = up_vec;
  buildCameraBasis(direction, up, horizontal);
}

auto OrthographicCamera::generateRay(Vec2f point) -> Ray {
  auto u = point.x();
  auto v = point.y();
  auto origin = center + (v - 0.5f) * size * up + (u - 0.5f) * size * horizontal;
  return Ray(origin, direction);
}

auto OrthographicCamera::getTMin() const -> float {
  return -1.0e30f;
}

PerspectiveCamera::PerspectiveCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float angle) {
  center = c;
  direction = dir;
  up = up_vec;
  buildCameraBasis(direction, up, horizontal);
  halfHeight = ::tanf(angle * 0.5f);
}

auto PerspectiveCamera::generateRay(Vec2f point) -> Ray {
  auto u = point.x();
  auto v = point.y();
  auto screenPoint = center + direction
      + (v - 0.5f) * 2.0f * halfHeight * up
      + (u - 0.5f) * 2.0f * halfHeight * horizontal;
  auto rayDir = screenPoint - center;
  rayDir.Normalize();
  return Ray(center, rayDir);
}

auto PerspectiveCamera::getTMin() const -> float {
  return 1e-4f;
}
