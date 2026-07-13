#include "vertical_force_field.h"

// 垂直力场
// 拉向 y = 0 平面，加速度 a = (0, -magnitude * y, 0)
Vec3f VerticalForceField::getAcceleration(const Vec3f &position, float /*mass*/, float /*t*/) const {
  return Vec3f(0, -magnitude * position.y(), 0);
}
