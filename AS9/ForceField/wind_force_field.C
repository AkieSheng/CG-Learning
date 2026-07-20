#include "wind_force_field.h"

#include <cassert>
#include <cmath>

auto WindForceField::getAcceleration(Vec3f const& position, float mass, float t) const -> Vec3f {
  assert(mass > 0);

  auto wx = ::sinf(2.0f * t + 0.5f * position.y());
  auto wz = ::cosf(1.5f * t + 0.3f * position.y());

  auto force = magnitude * Vec3f(wx, 0, wz);
  return force * (1.0f / mass);
}
