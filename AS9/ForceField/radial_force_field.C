#include "radial_force_field.h"

auto RadialForceField::getAcceleration(Vec3f const& position, float, float) const -> Vec3f {
  return -magnitude * position;
}
