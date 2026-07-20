#include "gravity_force_field.h"

auto GravityForceField::getAcceleration(Vec3f const& /*position*/, float /*mass*/,
                                        float /*t*/) const -> Vec3f {
  return gravity;
}
