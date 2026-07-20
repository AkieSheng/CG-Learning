#include "vertical_force_field.h"

auto VerticalForceField::getAcceleration(Vec3f const& position, float /*mass*/,
                                         float /*t*/) const -> Vec3f {
  return Vec3f(0, -magnitude * position.y(), 0);
}
