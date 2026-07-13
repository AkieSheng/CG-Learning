#ifndef _CONSTANT_FORCE_FIELD_H_
#define _CONSTANT_FORCE_FIELD_H_

#include "force_field.h"

// 恒定力场
class ConstantForceField : public ForceField {

public:
  ConstantForceField(Vec3f force) { this->force = force; }
  virtual ~ConstantForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;

private:
  Vec3f force;  // 恒定力
};

#endif
