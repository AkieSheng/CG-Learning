#include "constant_force_field.h"
#include <assert.h>

// 常力场
// 加速度 a = F / m，m > 0
Vec3f ConstantForceField::getAcceleration(const Vec3f & /*position*/, float mass, float /*t*/) const {
  assert(mass > 0);
  return force * (1.0f / mass);
}
