#ifndef _FORCE_FIELD_H_
#define _FORCE_FIELD_H_

#include "vectors.h"

// 力场基类
// 根据粒子位置 / 质量 / 时间返回加速度 a(p, m, t)
class ForceField {

public:
  ForceField() {}
  virtual ~ForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const = 0;
};

#endif
