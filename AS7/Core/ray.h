#ifndef _RAY_H
#define _RAY_H

#include <iostream>

#include "vectors.h"

// 射线
class Ray {

public:
  Ray() {}
  Ray(const Vec3f &orig, const Vec3f &dir) {
    origin = orig;
    direction = dir;
  }
  Ray(const Ray &r) { *this = r; }

  const Vec3f& getOrigin() const { return origin; }
  const Vec3f& getDirection() const { return direction; }
  Vec3f pointAtParameter(float t) const {  // 计算射线上的点
    return origin + direction * t;  // P(t) = origin + t * direction
  }

private:
  Vec3f origin;      // 射线起点
  Vec3f direction;   // 射线方向
};

// 输出射线
inline std::ostream &operator<<(std::ostream &os, const Ray &r) {
  os << "Ray <o:" << r.getOrigin() << ", d:" << r.getDirection() << ">";
  return os;
}

#endif
