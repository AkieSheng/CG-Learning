#include "camera.h"

OrthographicCamera::OrthographicCamera(Vec3f c, Vec3f dir, Vec3f up_vec, float s)
{
  center = c;
  size = s;

  direction = dir;
  direction /= direction.Length();

  up = up_vec;
  up -= direction * up.Dot3(direction);
  up /= up.Length();

  Vec3f::Cross3(horizontal, direction, up);
  horizontal /= horizontal.Length();
}

auto OrthographicCamera::generateRay(Vec2f point) -> Ray
{
  auto u = point.x();
  auto v = point.y();
  auto origin = center + (v - 0.5f) * size * up + (u - 0.5f) * size * horizontal;
  return Ray(origin, direction);
}

auto OrthographicCamera::getTMin() const -> float
{
  return -1.0e30f;
}
