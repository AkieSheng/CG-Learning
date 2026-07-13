#include "radial_force_field.h"

// 径向力场
// 始终指向原点，强度正比于到原点的距离，a(p) = -magnitude * p
// （|p|=1, |v|=5, magnitude=25 → |a|=v²/r，形成圆轨道）
Vec3f RadialForceField::getAcceleration(const Vec3f &position, float /*mass*/, float /*t*/) const {
  return -magnitude * position;
}
