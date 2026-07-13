#ifndef _GRAVITY_FORCE_FIELD_H_
#define _GRAVITY_FORCE_FIELD_H_

#include "force_field.h"

// 重力场
class GravityForceField : public ForceField {

public:
  GravityForceField(Vec3f gravity) { this->gravity = gravity; }
  virtual ~GravityForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;

private:
  Vec3f gravity;  // 重力加速度
};

#endif
