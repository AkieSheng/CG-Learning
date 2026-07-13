#ifndef _RADIAL_FORCE_FIELD_H_
#define _RADIAL_FORCE_FIELD_H_

#include "force_field.h"

// 径向力场
class RadialForceField : public ForceField {

public:
  RadialForceField(float magnitude) { this->magnitude = magnitude; }
  virtual ~RadialForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;

private:
  float magnitude;  // 径向力场强度
};

#endif
