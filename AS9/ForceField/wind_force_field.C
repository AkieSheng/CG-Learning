#include "wind_force_field.h"
#include <assert.h>
#include <math.h>

// 风场
// 水平方向主导，强度随时间与高度变化
Vec3f WindForceField::getAcceleration(const Vec3f &position, float mass, float t) const {
  assert(mass > 0);

  // 风向随时间摆动，叠加高度使上下段受力不同，形成弯曲
  float wx = sinf(2.0f * t + 0.5f * position.y());
  float wz = cosf(1.5f * t + 0.3f * position.y());

  Vec3f force = magnitude * Vec3f(wx, 0, wz);  // F = magnitude * (sin(...), 0, cos(...))
  return force * (1.0f / mass);  // a = F / m
}
