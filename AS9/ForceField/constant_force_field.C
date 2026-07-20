#include "constant_force_field.h"

#include <cassert>

auto ConstantForceField::getAcceleration(Vec3f const& /*position*/, float mass,
                                         float /*t*/) const -> Vec3f {
  assert(mass > 0);
  return force * (1.0f / mass);
}
