#ifndef _PLANE_H_
#define _PLANE_H_

#include "object3d.h"
#include "vectors.h"

// 无限平面图元，方程 P·n = d
class Plane : public Object3D {

public:
  Plane(Vec3f normal, float d, Material *m)
    : normal(normal), d(d) {
    material = m;
    normal.Normalize();
  }

  virtual bool intersect(const Ray &r, Hit &h, float tmin);
  virtual bool intersectShadow(const Ray &r, float tmin, float tmax, float &t,
                               Material **outMaterial);
  virtual void paint(void) const;

private:
  Vec3f normal;  // 平面法线（单位向量）
  float d;  // 原点到平面的距离（沿法线方向，有符号）
};

#endif
