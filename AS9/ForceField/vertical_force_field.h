#ifndef _VERTICAL_FORCE_FIELD_H_
#define _VERTICAL_FORCE_FIELD_H_

#include "force_field.h"

// 垂直力场
class VerticalForceField : public ForceField {

public:
  VerticalForceField(float magnitude) { this->magnitude = magnitude; }
  virtual ~VerticalForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;

private:
  float magnitude;  // 垂直力场强度
};

#endif
