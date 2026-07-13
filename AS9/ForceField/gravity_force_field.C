#include "gravity_force_field.h"

// 重力场
// 加速度 a = g
Vec3f GravityForceField::getAcceleration(const Vec3f & /*position*/, float /*mass*/, float /*t*/) const {
  return gravity;
}
