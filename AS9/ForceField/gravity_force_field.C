#include "gravity_force_field.h"

auto GravityForceField::getAcceleration(Vec3f const&, float, float) const -> Vec3f {
  return gravity;
}
