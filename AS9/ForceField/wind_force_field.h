#ifndef _WIND_FORCE_FIELD_H_
#define _WIND_FORCE_FIELD_H_

#include "force_field.h"

// 风场
class WindForceField : public ForceField {

public:
  WindForceField(float magnitude) { this->magnitude = magnitude; }
  virtual ~WindForceField() {}

  virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;

private:
  float magnitude;  // 风力强度
};

#endif
